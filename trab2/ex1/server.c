#include "common.h"
#include <unistd.h>

int main() {
  pid_t pid = fork();

  if (pid == 0) {
    // FILHO → UNIX
    int sock_unix;
    sock_unix = create_unix_socket();
    listen(sock_unix, 5);
    printf("[UNIX]: Listening on path \"%s\"\n", SOCKET_PATH);

    int clientfd;

    while (1) {
      clientfd = accept(sock_unix, NULL, NULL);
      printf("[UNIX]: Received client on fd:%d\n", clientfd);

      handle_client(clientfd);
      printf("[UNIX]: Client on fd:%d handled\n", clientfd);

      close(clientfd);
    }
    close(sock_unix);
		unlink(SOCKET_PATH);
    printf("[UNIX]: Closed Unix socket\n");
  } else {
    // PAI → INET
    int sock_inet;
    sock_inet = create_inet_socket();
    listen(sock_inet, 5);
    printf("[INET]: Listening on port %d\n", SERVER_PORT);

    int clientfd;
    while (1) {
      clientfd = accept(sock_inet, NULL, NULL);
      printf("[INET]: Received client on fd:%d\n", clientfd);

      handle_client(clientfd);
      printf("[INET]: Client on fd:%d handled\n", clientfd);

      close(clientfd);
    }
    close(sock_inet);
    printf("[INET]: Closed Inet socket\n");
  }

  return 0;
}
