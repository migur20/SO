#include "common.h"
#include <unistd.h>

int main() {
  int sock_inet;
  sock_inet = create_inet_socket();
  listen(sock_inet, 5);
  printf("Listening on port %d\n", SERVER_PORT);

  int clientfd;
  while (1) {
    clientfd = accept(sock_inet, NULL, NULL);
    printf("Received client on fd:%d\n", clientfd);

    handle_client(clientfd);
    printf("Client on fd:%d handled\n", clientfd);

    close(clientfd);
  }
  close(sock_inet);
  printf("Closed Inet socket\n");

  return 0;
}
