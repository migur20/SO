#include "common.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define VALUES_PER_THREAD 1000

typedef struct {
  int clientfd;
} ThreadArgs;

/* Recebe os dados do cliente e envia o status e resultado do processamento */
int handle_client(int clientfd) {
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

  // if (dim / sizeof(*values) < 50) {
  //   for (uint32_t i = 0; i < dim / sizeof(*values); i++) {
  //     printf("%d, ", values[i]);
  //   }
  //   putchar('\n');
  // }

  // Tenta enviar o status ate conseguir
  while (send_status(clientfd, OK, NULL) == EXIT_FAILURE) {
    perror("write status (resending...)");
  }

  // Processar os values ...
  int nthreads = (dim + VALUES_PER_THREAD - 1) / VALUES_PER_THREAD;
  if (nthreads < 2)
    nthreads = 2;

  ThreadReturn ret = values_processing(values, dim/sizeof(*values), nthreads);

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

void *server_func(void *_args) {
  ThreadArgs *args = (ThreadArgs *)_args;

  handle_client(args->clientfd);
  printf("Client on fd:%d handled\n", args->clientfd);

  close(args->clientfd);
  return NULL;
}

int main(int argc, char *argv[]) {
	if(argc != 2){
		fprintf(stderr, "Uso: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);

  int sock_inet;
  sock_inet = create_inet_socket(port);
  listen(sock_inet, 5);
  printf("Listening on port %d\n", port);

  int clientfd;
  while (1) {
    clientfd = accept(sock_inet, NULL, NULL);
    printf("Received client on fd:%d\n", clientfd);
    pthread_t thread;
    pthread_create(&thread, NULL, server_func,
                   (void *)&(ThreadArgs){
                       .clientfd = clientfd,
                   });
    pthread_detach(thread);
  }

  close(sock_inet);
  printf("Closed Inet socket\n");

  return 0;
}
