#include "common.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

  if (dim / sizeof(*values) < 50) {
    for (uint32_t i = 0; i < dim / sizeof(*values); i++) {
      printf("%d, ", values[i]);
    }
    putchar('\n');
  }

	// Tenta enviar o status ate conseguir
  while(send_status(clientfd, OK, NULL) == EXIT_FAILURE){
		perror("write status (resending...)");
	}

	// Processar os values ...
	// TODO


  uint16_t min = 13, max = 31;
  uint64_t sum = 45;

  if (send_data(clientfd, &min, sizeof(min)) == EXIT_FAILURE) {
    perror("write min");
    return EXIT_FAILURE;
  }
  if (send_data(clientfd, &max, sizeof(max)) == EXIT_FAILURE) {
    perror("write max");
    return EXIT_FAILURE;
  }
  if (send_data(clientfd, &sum, sizeof(sum)) == EXIT_FAILURE) {
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

int main() {
  int sock_inet;
  sock_inet = create_inet_socket();
  listen(sock_inet, 5);
  printf("Listening on port %d\n", SERVER_PORT);

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
