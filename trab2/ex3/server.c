#include "common.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
  int clientfd;
} ThreadArgs;

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
  }

  close(sock_inet);
  printf("Closed Inet socket\n");

  return 0;
}
