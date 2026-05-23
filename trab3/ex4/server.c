#include "common.h"
#include "threadpool.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define CON_TIMEOUT 10

#define INET 0
#define UNIX 1

#define PRINT_PERIOD 3 // in seconds

#define QEUEU_SIZE 5
#define NTHREADS 5

#define VALUES_PER_THREAD 1000

typedef struct {
  uint16_t *v; // pointer posicao inicial
  unsigned long dim;
} ThreadArgs;

typedef struct {
  uint64_t sum;
  uint16_t bigger;
  uint16_t smaller;
} ThreadReturn;

typedef struct {
  int n_con[2];
  int total_dim;
  bool shutdown;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool stats_changed;
} ServerStats;

typedef struct {
  ServerStats *stats;
  int socket_fd;
  const char *con_name;
  int con_type;
} ServerThreadArgs;

typedef struct {
  int clientfd;
  ServerStats *stats;
} ClientHandlerArgs;

void *thread_func(void *_args)
{
  ThreadArgs args = *(ThreadArgs *)_args;
  ThreadReturn *ret = calloc(1, sizeof(*ret)); // inicializa a 0
  *ret = (ThreadReturn){
      .sum = 0,
      .bigger = args.v[0],
      .smaller = args.v[0],
  };

  for (size_t i = 0; i < args.dim; ++i) {
    ret->sum += args.v[i];
    if (args.v[i] > ret->bigger)
      ret->bigger = args.v[i];
    if (args.v[i] < ret->smaller)
      ret->smaller = args.v[i];
  }

  return ret;
}

void stats_add_dim(ServerStats *stats, int dim)
{
  pthread_mutex_lock(&stats->mutex);
  stats->stats_changed = true;
  stats->total_dim += dim;
  pthread_cond_broadcast(&stats->cond);
  pthread_mutex_unlock(&stats->mutex);
}

void stats_add_con(ServerStats *stats, int con_type)
{
  pthread_mutex_lock(&stats->mutex);
  stats->stats_changed = true;
  stats->n_con[con_type]++;
  pthread_cond_broadcast(&stats->cond);
  pthread_mutex_unlock(&stats->mutex);
}

ThreadReturn values_processing(uint16_t *values, uint32_t dim, int nthreads)
{
  int resto = dim % nthreads;
  dim = dim / nthreads;

  pthread_t th[nthreads];
  ThreadArgs args[nthreads];
  for (int i = 0; i < nthreads; i++) {
    args[i].v = values + (i * dim);
    args[i].dim = (i == nthreads - 1) ? dim + resto : dim;
    if (pthread_create(&th[i], NULL, thread_func, &args[i]) != 0)
      fatal_system_error("criar thread processamento");
  }

  long sum = 0;
  int bigger = values[0];
  int smaller = values[0];

  for (int i = 0; i < nthreads; i++) {
    ThreadReturn *ret;
    pthread_join(th[i], (void **)&ret);
    sum += ret->sum;
    if (ret->smaller < smaller)
      smaller = ret->smaller;
    if (ret->bigger > bigger)
      bigger = ret->bigger;
    free(ret);
  }

  return (ThreadReturn){.sum = sum, .bigger = bigger, .smaller = smaller};
}

void *handle_client(void *_args)
{
  // Copia local de clientfd, nao e um pointer para o valor original!!!
  int clientfd = ((ClientHandlerArgs *)_args)->clientfd;
  ServerStats *stats = ((ClientHandlerArgs *)_args)->stats;

  uint32_t dim = 0; // EM BYTES!!!!!!!!!!!!!!!!!1
  if (receive_data(clientfd, &dim, sizeof(dim)) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler dim") == EXIT_FAILURE)
      perror("write status");
    return NULL;
  }

  uint16_t *values = calloc(1, dim);

  stats_add_dim(stats, dim/sizeof(*values));

  int timeout = 0;
  while (receive_data(clientfd, values, dim) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler values") == EXIT_FAILURE)
      perror("write status");
    if (timeout == CON_TIMEOUT) {
      fprintf(stderr, "Connection timeout!!!\n");
      close(clientfd);
      return NULL;
    }
    fprintf(stderr, "retrying...\n");
    timeout++;
  }

  // Tenta enviar o status ate conseguir
  timeout = 0;
  while (send_status(clientfd, OK, NULL) == EXIT_FAILURE) {
    perror("write status");
    if (timeout == CON_TIMEOUT) {
      fprintf(stderr, "Connection timeout!!!\n");
      close(clientfd);
      return NULL;
    }
    fprintf(stderr, "retrying...\n");
    timeout++;
  }

  // Processar os values
  int nthreads = (dim + VALUES_PER_THREAD - 1) / VALUES_PER_THREAD;
  if (nthreads < 2)
    nthreads = 2;

  ThreadReturn ret = values_processing(values, dim / sizeof(*values), nthreads);
  free(values);

  if (send_data(clientfd, &ret.smaller, sizeof(ret.smaller)) == EXIT_FAILURE) {
    perror("write smaller");
    return NULL;
  }
  if (send_data(clientfd, &ret.bigger, sizeof(ret.bigger)) == EXIT_FAILURE) {
    perror("write bigger");
    return NULL;
  }
  if (send_data(clientfd, &ret.sum, sizeof(ret.sum)) == EXIT_FAILURE) {
    perror("write sum");
    return NULL;
  }

  close(clientfd);
  return NULL;
}

void print_stats(ServerStats *stats)
{
  printf("[LOG]Unix Connections: %d\n", stats->n_con[UNIX]);
  printf("[LOG]Inet Connections: %d\n", stats->n_con[INET]);
  printf("[LOG]Average vector dimension: %d\n",
         (stats->total_dim == 0)
             ? 0
             : (stats->total_dim) / (stats->n_con[INET] + stats->n_con[UNIX]));
}

void *stats_func_periodic(void *_args)
{
  ServerStats *stats = _args;
  while (1) {
    sleep(PRINT_PERIOD);
    pthread_mutex_lock(&stats->mutex);
    if (stats->shutdown) {
      pthread_mutex_unlock(&stats->mutex);
      return NULL;
    }
    pthread_mutex_unlock(&stats->mutex);
		print_stats(stats);
  }
  perror("Unreachable");
  return NULL;
}

void *stats_func_wait(void *_args)
{
  ServerStats *stats = _args;
  while (1) {
    // sleep(PRINT_PERIOD);
    pthread_mutex_lock(&stats->mutex);
    while (!stats->stats_changed && !stats->shutdown)
      pthread_cond_wait(&stats->cond, &stats->mutex);
    if (stats->shutdown) {
      pthread_mutex_unlock(&stats->mutex);
      return NULL;
    }
    stats->stats_changed = false;
    pthread_mutex_unlock(&stats->mutex);

		print_stats(stats);
  }
  perror("Unreachable");
  return NULL;
}

void *server_func(void *_args)
{
  ServerThreadArgs args = *(ServerThreadArgs *)_args;
  ServerStats *stats = args.stats;
  threadpool_t tp = {0};
  threadpool_init(&tp, QEUEU_SIZE, NTHREADS);

  listen(args.socket_fd, QEUEU_SIZE);

  while (true) {
    int client_fd = accept(args.socket_fd, NULL, NULL);
    if (client_fd == -1) {
      if (stats->shutdown) {
        break;
      }
      fprintf(stderr, "%s accept:", args.con_name);
      perror("");
      continue;
    }

    printf("[%s]Received client on fd:%d\n", args.con_name, client_fd);
    // client_fd e copiado para uma variavel local em handle_client
    ClientHandlerArgs ch_args = {
        .clientfd = client_fd,
        .stats = stats,
    };
    threadpool_submit(&tp, handle_client, &ch_args);

    stats_add_con(stats, args.con_type);
  }
  threadpool_destroy(&tp);
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Uso: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = atoi(argv[1]);

  pthread_t th_stats;
  pthread_t th_inet;
  pthread_t th_unix;

  ServerStats stats = {
      .total_dim = 0,
  };
  pthread_mutex_init(&stats.mutex, NULL);
  pthread_cond_init(&stats.cond, NULL);

  pthread_create(&th_stats, NULL, stats_func_periodic, &stats);

  ServerThreadArgs inet_args = {
      .stats = &stats,
      .socket_fd = create_inet_socket(port),
      .con_type = INET,
      .con_name = "INET",
  };
  ServerThreadArgs unix_args = {
      .stats = &stats,
      .socket_fd = create_unix_socket(),
      .con_type = UNIX,
      .con_name = "UNIX",
  };
  pthread_create(&th_inet, NULL, server_func, &inet_args);
  pthread_create(&th_unix, NULL, server_func, &unix_args);

  char in;
  while (1) {
    in = getchar();
    if (in == 'q') {
      pthread_mutex_lock(&stats.mutex);
      stats.shutdown = true;
      pthread_cond_broadcast(&stats.cond);
      pthread_mutex_unlock(&stats.mutex);
      break;
    }
  }

  printf("Waiting for inet thread to end...\n");
  shutdown(inet_args.socket_fd, SHUT_RDWR);
  close(inet_args.socket_fd);
  pthread_join(th_inet, NULL);
  printf("Inet thread finished\n");

  printf("Waiting for unix thread to end...\n");
  shutdown(unix_args.socket_fd, SHUT_RDWR);
  close(unix_args.socket_fd);
  pthread_join(th_unix, NULL);
  printf("Unix thread finished\n");

  printf("Waiting for stats thread to end...\n");
  pthread_join(th_stats, NULL);
  printf("Stats thread finished\n");

  pthread_mutex_destroy(&stats.mutex);
	pthread_cond_destroy(&stats.cond);
  return EXIT_SUCCESS;
}
