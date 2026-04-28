#include "common.h"
#include <stdint.h>

/* Função principal do cliente */
int main(int argc, char *argv[]) {
  int sockfd;
  struct sockaddr_un addr;
  uint8_t service;
  uint8_t status;

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

  /* Envia o código do serviço */
  if (write(sockfd, &service, sizeof(service)) == -1)
    fatal_system_error("write enviar codigo servico");

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
      fatal_system_error("read tamanho dos dados");

    if (size == 0)
      break;

    uint32_t remaining = size;
    while (remaining > 0) {
      uint32_t to_read = remaining < BUF_SIZE ? remaining : BUF_SIZE;
      int n = read(sockfd, buffer, to_read);
      if (n <= 0)
        fatal_system_error("read dados do bloco");
      write(STDOUT_FILENO, buffer, n);
      remaining -= n;
    }
  }

  close(sockfd);

  return 0;
}
