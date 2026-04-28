#include "common.h"
#include <stdio.h>

/* Servidor principal */
int main() {
  int sock_inet;
  int clientfd;

  sock_inet = create_inet_socket();

  if (listen(sock_inet, 5) == -1)
    fatal_system_error("listen");

  printf("Listening on port %d\n", SERVER_PORT);

  while (1) {
    clientfd = accept(sock_inet, NULL, NULL);
		printf("Received client on fd:%d\n", clientfd);

    handle_client(clientfd);
		printf("Client on fd:%d handled\n", clientfd);

    close(clientfd);
  }

  close(sock_inet);

  return 0;
}
