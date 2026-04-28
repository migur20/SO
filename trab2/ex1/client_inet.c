#include "common.h"
#include <stdio.h>
/* Cliente INET/TCP */
int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_in addr;
  uint8_t service;
  uint8_t status;

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

  /* Envia o código do serviço */
  if (write(sockfd, &service, sizeof(service)) == -1)
    fatal_system_error("write codigo do servico");

  /* Recebe o estado da resposta */
  if (read(sockfd, &status, sizeof(status)) == -1)
    fatal_system_error("read status do servidor");
  if (status == INVALID_SERVICE) {
    close(sockfd);
    fatal_system_error("Serviço inválido\n");
  }

  uint32_t size;
  char buffer[BUF_SIZE];

  while (1) {
    if (read(sockfd, &size, sizeof(size)) == -1)
      fatal_system_error("read tamanho do bloco");

		size = htonl(size);

    if (size == 0)
      break;

    while (size > 0) {
      uint32_t bytes_to_read = size < BUF_SIZE ? size : BUF_SIZE;
      int n = read(sockfd, buffer, bytes_to_read);
      if (n <= 0)
        fatal_system_error("read dados do bloco");
      write(STDOUT_FILENO, buffer, n);
      size -= n;
    }
  }

  close(sockfd);

  return 0;
}
