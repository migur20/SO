#include "common.h"

int main() {
  int sock_unix;

  sock_unix = create_unix_socket();

  if (listen(sock_unix, 5) == -1)
    fatal_system_error("listen");

	printf("Listening on path \"%s\"\n", SOCKET_PATH);

  while (1) {
    int clientfd = accept(sock_unix, NULL, NULL);
		printf("Received client on fd:%d\n", clientfd);

    handle_client(clientfd);
		printf("Client on fd:%d handled\n", clientfd);

    close(clientfd);
  }

  close(sock_unix);
  unlink(SOCKET_PATH);

  return 0;
}
