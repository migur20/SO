#include "common.h"

int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in addr;
  uint8_t service;

  if (argc != 3) {
    printf("Uso: %s <ip_servidor> <1=lscpu | 2=free -h>\n", argv[0]);
    return 1;
  }

  service = atoi(argv[2]);

  /* Cria o socket TCP */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* Define o IP e o porto do servidor */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addr.sin_port = htons(SERVER_PORT);

  /* Liga ao servidor */
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("connect");

	client_protocol(sockfd, service);

  return 0;
}
