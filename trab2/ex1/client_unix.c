#include "common.h"

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_un addr;
  uint8_t service;

  if (argc != 2) {
    printf("Uso: %s <1=lscpu | 2=free -h>\n", argv[0]);
    return 1;
  }

  service = atoi(argv[1]);

  /* Cria o socket UNIX do cliente */
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  /* Define o endereço do servidor */
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SOCKET_PATH);

  /* Liga o cliente ao servidor */
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("connect");

	client_protocol(sockfd, service);

  close(sockfd);

  return 0;
}
