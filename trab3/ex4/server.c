#include "common.h"
#include "threadpool.h"
#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

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
  pthread_mutex_t mutex;
  bool shutdown;
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
  for (unsigned long j = 0; j < args.dim; ++j) {
    ret->sum += args.v[j];
    if (args.v[j] > ret->bigger)
      ret->bigger = args.v[j];
    if (args.v[j] < ret->smaller)
      ret->smaller = args.v[j];
  }
  return ret;
}

ThreadReturn values_processing(uint16_t *values, uint32_t dim, int nthreads)
{
  int resto = dim % nthreads;
  dim = dim / nthreads;

  pthread_t th[nthreads];
  // printf("dim per th:%d, nthreads: %d\n", dim, nthreads);
  for (int i = 0; i < nthreads; i++) {
    ThreadArgs args = {0};
    args.v = values + (i * dim);
    args.dim = (i == nthreads - 1) ? dim + resto : dim;
    if (pthread_create(&th[i], NULL, thread_func, &args) != 0)
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

  uint32_t dim;
  if (receive_data(clientfd, &dim, sizeof(dim)) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler dim") == EXIT_FAILURE)
      perror("write status");
    return NULL;
  }

  pthread_mutex_lock(&stats->mutex);
  stats->total_dim += dim;
  pthread_mutex_unlock(&stats->mutex);

  uint16_t *values;
  values = malloc(dim);
  if (receive_data(clientfd, values, dim) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler values") == EXIT_FAILURE)
      perror("write status");
    return NULL;
  }

  for (size_t i = 0; i < dim; i++)
    printf("values[%zu]: %d\n", i, values[i]);

  // Tenta enviar o status ate conseguir
  while (send_status(clientfd, OK, NULL) == EXIT_FAILURE) {
    perror("write status (resending...)");
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
  return NULL;
}

void *stats_func(void *_args)
{
  ServerStats *args = _args;
  printf("stats stats_func: %p\n", args);
  bool isRunning = true;
  while (isRunning) {
    sleep(PRINT_PERIOD);
    pthread_mutex_lock(&args->mutex);
    int n_con_inet = args->n_con[INET];
    int n_con_unix = args->n_con[UNIX];
    printf("Unix Connections: %d\n", args->n_con[UNIX]);
    printf("Inet Connections: %d\n", args->n_con[INET]);
    printf("total: %d, Average vector dimension: %.2f\n", args->total_dim,
           (args->total_dim == 0)
               ? 0.0
               : ((float)args->total_dim) / (n_con_inet + n_con_unix));
    if (args->shutdown)
      isRunning = false;
    pthread_mutex_unlock(&args->mutex);
  }
  return NULL;
}

void *server_func(void *_args)
{
  ServerThreadArgs args = *(ServerThreadArgs *)_args;
  ServerStats *stats = args.stats;
  // printf("args: %p, %s, %d, %d\n", args.stats, args.con_name, args.con_type,
  //        args.socket_fd);
  threadpool_t tp = {0};
  threadpool_init(&tp, QEUEU_SIZE, NTHREADS);

  listen(args.socket_fd, QEUEU_SIZE);

  bool isRunning = true;
  while (isRunning) {
    int client_fd = accept(args.socket_fd, NULL, NULL);
    if (client_fd == -1) {
      fprintf(stderr, "%s accept\n", args.con_name);
      perror("accept");
      printf("socketfd: %d\n", args.socket_fd);
      return NULL;
      continue;
    }
    printf("[%s]Received client on fd:%d\n", args.con_name, client_fd);
    // client_fd e copiado para uma variavel local em handle_client
    ClientHandlerArgs ch_args = {
        .clientfd = client_fd,
        .stats = stats,
    };
    threadpool_submit(&tp, handle_client, &ch_args);

    pthread_mutex_lock(&stats->mutex);
    stats->n_con[args.con_type]++;
    pthread_mutex_unlock(&stats->mutex);
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

  printf("stats main: %p\n", &stats);

  pthread_create(&th_stats, NULL, stats_func, &stats);

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
      pthread_mutex_unlock(&stats.mutex);
      break;
    }
  }

  shutdown(inet_args.socket_fd, SHUT_RDWR);
  close(inet_args.socket_fd);
  pthread_join(th_inet, NULL);

  shutdown(unix_args.socket_fd, SHUT_RDWR);
  close(unix_args.socket_fd);
  pthread_join(th_unix, NULL);

  pthread_join(th_stats, NULL);

  pthread_mutex_destroy(&stats.mutex);
  return EXIT_SUCCESS;
}
