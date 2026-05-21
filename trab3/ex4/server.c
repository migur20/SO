#include "common.h"
#include "threadpool.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  int n_unix_con;
  int n_inet_con;
  int average_dim;
  pthread_mutex_t mutex;
  bool shutdown;
} ServerStats;

#define PRINT_FREQ 1 // in seconds

void *stats_func(void *_args)
{
  ServerStats *args = _args;
  bool isRunning = true;
  while (isRunning) {
    sleep(PRINT_FREQ);
    pthread_mutex_lock(&args->mutex);
    printf("Unix Connections: %d\n", args->n_unix_con);
    printf("Inet Connections: %d\n", args->n_inet_con);
    printf("Average vector dimension: %d\n", args->average_dim);
    if (args->shutdown)
      isRunning = false;
    pthread_mutex_unlock(&args->mutex);
  }
  return NULL;
}

typedef struct {
  ServerStats *stats;
  int port;
} ServerThreadArgs;

#define QEUEU_SIZE 5
#define NTHREADS 5

typedef struct {
  uint16_t *v; // pointer posicao inicial
  unsigned long dim;
} ThreadArgs;

typedef struct {
  uint64_t sum;
  uint16_t bigger;
  uint16_t smaller;
} ThreadReturn;

#define VALUES_PER_THREAD 1000

void *thread_func(void *_args)
{
  ThreadArgs *args = (ThreadArgs *)_args;
  ThreadReturn *ret = calloc(1, sizeof(*ret)); // inicializa a 0
  *ret = (ThreadReturn){
      .sum = 0,
      .bigger = args->v[0],
      .smaller = args->v[0],
  };
  for (unsigned long j = 0; j < args->dim; ++j) {
    ret->sum += args->v[j];
    if (args->v[j] > ret->bigger)
      ret->bigger = args->v[j];
    if (args->v[j] < ret->smaller)
      ret->smaller = args->v[j];
  }
  // printf("smaller:%d, bigger:%d, sum:%ld\n", ret->smaller, ret->bigger,
  // ret->sum);
  return ret;
}

ThreadReturn values_processing(uint16_t *values, uint32_t dim, int nthreads)
{
  int resto = dim % nthreads;
  dim = dim / nthreads;

  ThreadArgs args[nthreads];
  pthread_t th[nthreads];
  // printf("dim per th:%d, nthreads: %d\n", dim, nthreads);
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
  int clientfd = *(int *)_args;

  uint32_t dim;
  if (receive_data(clientfd, &dim, sizeof(dim)) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler dim") == EXIT_FAILURE)
      perror("write status");
    return EXIT_FAILURE;
  }
  uint16_t *values;
  values = malloc(dim);
  if (receive_data(clientfd, values, dim) == EXIT_FAILURE) {
    perror("read values");
    if (send_status(clientfd, EXEC_ERROR, "erro ao ler values") == EXIT_FAILURE)
      perror("write status");
    return EXIT_FAILURE;
  }

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
    return EXIT_FAILURE;
  }
  if (send_data(clientfd, &ret.bigger, sizeof(ret.bigger)) == EXIT_FAILURE) {
    perror("write bigger");
    return EXIT_FAILURE;
  }
  if (send_data(clientfd, &ret.sum, sizeof(ret.sum)) == EXIT_FAILURE) {
    perror("write sum");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void *inet_func(void *_args)
{
  ServerThreadArgs *args = _args;
  threadpool_t tp = {0};
  threadpool_init(&tp, QEUEU_SIZE, NTHREADS);
  int socket_fd = create_inet_socket(args->port);
  bool isRunning = true;
  while (isRunning) {
    int client_fd = accept(socket_fd, NULL, NULL);
    if (client_fd == -1) {
      perror("inet accept");
      continue;
    }
    printf("[INET]Received client on fd:%d\n", client_fd);
    // client_fd e copiado para uma variavel local em handle_client
    threadpool_submit(&tp, handle_client, &client_fd);
  }
  threadpool_destroy(&tp);
  return NULL;
}

void *unix_func(void *_args)
{
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

  ServerStats stats = {0};
  pthread_mutex_init(&stats.mutex, NULL);

  pthread_create(&th_stats, NULL, stats_func, &stats);

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

  pthread_join(th_stats, NULL);
  pthread_join(th_unix, NULL);
  pthread_join(th_inet, NULL);

  pthread_mutex_destroy(&stats.mutex);
  return EXIT_SUCCESS;
}
