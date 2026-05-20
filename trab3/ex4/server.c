#include "common.h"
#include "threadpool.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define VALUES_PER_THREAD 1000

#define QUEUE_SIZE 5
#define N_THREADS 5

typedef struct {
  int clientfd;
} ThreadArgs;

typedef struct {
  int n_unix_con;
  int n_inet_con;
  int average_dim;
} ServerStats;

void *print_statistcs(void *_args)
{
  ServerStats *args = _args;
	int n_cons = args->n_inet_con + args->n_unix_con;
  while (1) {
    if (args->n_inet_con + args->n_unix_con > n_cons) {
      printf(
          "Unix Connections: %d\nInet Connections: %d\nAverage Dimension: %d\n",
          args->n_unix_con, args->n_inet_con, args->average_dim);
			n_cons = args->n_inet_con + args->n_unix_con;
    }
		sleep(1);
  }
  return NULL;
}

/* Recebe os dados do cliente e envia o status e resultado do processamento */
int handle_client(int clientfd)
{
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

  // Processar os values ...
  int nthreads = (dim + VALUES_PER_THREAD - 1) / VALUES_PER_THREAD;
  if (nthreads < 2)
    nthreads = 2;

  ThreadReturn ret = values_processing(values, dim / sizeof(*values), nthreads);

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

void *handle_client_thread_func(void *_args)
{
  ThreadArgs *args = (ThreadArgs *)_args;

  handle_client(args->clientfd);
  printf("Client on fd:%d handled\n", args->clientfd);

  close(args->clientfd);
  return NULL;
}

void *server_thread_func(void *_args)
{
  ServerStats *args = _args;

  handle_client(args->clientfd);
  printf("Client on fd:%d handled\n", args->clientfd);

  close(args->clientfd);
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Uso: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  int port = atoi(argv[1]);

  int sock_inet = create_inet_socket(port);
  int sock_unix = create_unix_socket();

  listen(sock_inet, QUEUE_SIZE);
  listen(sock_unix, QUEUE_SIZE);
  printf("Listening on port %d\n", port);

  threadpool_t tp = {0};
  threadpool_init(&tp, QUEUE_SIZE, N_THREADS);

	ServerStats stats = {0};

  int clientfd;
  while (1) {
    clientfd = accept(sock_inet, NULL, NULL);
    if (clientfd == -1) {
      perror("accept");
      continue;
    }
    printf("Received client on fd:%d\n", clientfd);
    pthread_t thread;
    ThreadArgs *args = malloc(sizeof(*args));
    if (!args) {
      perror("malloc");
      close(clientfd);
      continue;
    }
    args->clientfd = clientfd;
    threadpool_submit(&tp, handle_client_thread_func, args);
  }

  threadpool_destroy(&tp);

  close(sock_inet);
  printf("Closed Inet socket\n");

  return 0;
}
