#include "common.h"
#include <stdint.h>
#include <stdio.h>

void fatal_system_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Envia um bloco no formato: [4 bytes dimensão][dados] */
void send_block(int fd, char *buffer, uint32_t size) {
	uint32_t net_size = htonl(size);
  write(fd, &net_size, sizeof(net_size));
  write(fd, buffer, size);
}

void send_status(int clientfd, uint8_t status){
	write(clientfd, &status, sizeof(status));
}

void run_service(int clientfd, uint8_t service) {



}

/* Recebe o pedido do cliente e valida o serviço pedido, e executa o servico */
void handle_client(int clientfd) {
  uint8_t service;

  read(clientfd, &service, sizeof(service));
  printf("Received service: %s\n", service == CPUINFO   ? "CPUINFO"
                                   : service == MEMINFO ? "MEMINFO"
                                                        : "UNKNOWN");
  if (service != CPUINFO && service != MEMINFO) {
    uint8_t status = INVALID_SERVICE;
    write(clientfd, &status, sizeof(status));
    return;
  }

  run_service(clientfd, service);
}

/* Cria o socket UNIX do servidor e faz o bind ao pathname definido */
int create_unix_socket() {
  int sockfd;
  struct sockaddr_un addr;

  unlink(SOCKET_PATH);

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1)
    fatal_system_error("criar socket(unix)");

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SOCKET_PATH);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind(unix)");

  return sockfd;
}

/* Cria o socket INET/TCP do servidor */
int create_inet_socket() {
  int sockfd;
  struct sockaddr_in addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind (inet)");

  return sockfd;
}
