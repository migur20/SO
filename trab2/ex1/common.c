#include "common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void fatal_system_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int receive_data(int fd, void *buffer, size_t size) {
  size_t bytes_read = 0;
  while (bytes_read < size) {
    int n = read(fd, buffer + bytes_read, size - bytes_read);
    if (n <= 0)
      return EXIT_FAILURE;
    bytes_read += n;
  }
  return EXIT_SUCCESS;
}

int send_data(int fd, void *buffer, size_t size) {
  size_t bytes_written = 0;
  while (bytes_written < size) {
    int n = write(fd, buffer + bytes_written, size - bytes_written);
    if (n <= 0)
      return EXIT_FAILURE;
    bytes_written += n;
  }
  return EXIT_SUCCESS;
}

/* Envia um bloco no formato: [4 bytes dimensão][dados] */
int send_block(int fd, void *buffer, uint32_t size) {
  if (send_data(fd, &size, sizeof(size)) == EXIT_FAILURE)
    return EXIT_FAILURE;
  return send_data(fd, buffer, size);
}

int send_status(int clientfd, uint8_t status) {
  if (send_data(clientfd, &status, sizeof(status)) == EXIT_FAILURE)
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}

/* Executa o serviço pedido: 1 -> lscpu; 2 -> free -h */
void run_service(int clientfd, uint8_t service) {
  int p[2];
  if (pipe(p) == -1) {
    send_status(clientfd, EXEC_ERROR);
    perror("pipe");
    return;
  }

  int pid = fork();

  if (pid == -1) {
    send_status(clientfd, EXEC_ERROR);
    close(p[0]);
    close(p[1]);
    perror("forking");
    return;
  }

  /* Processo filho: redireciona o stdout para o pipe e executa o comando pedido
   */
  if (pid == 0) {

    close(p[0]);

    dup2(p[1], STDOUT_FILENO);

    close(p[1]);

    if (service == CPUINFO) {
      execlp("lscpu", "lscpu", NULL);
    } else {
      execlp("free", "free", "-h", NULL);
    }

    _exit(1);
  }
  /* Processo pai: lê o output do comando através do pipee envia esse output ao
   * cliente em blocos */
  close(p[1]);

  uint8_t status = OK;
  if (send_status(clientfd, status) == EXIT_FAILURE) {
    perror("write status");
    return;
  }

  char buffer[BUF_SIZE];
  int bytes_read = 0;
  int n;

  while ((n = read(p[0], buffer + bytes_read, BUF_SIZE - bytes_read)) > 0) {
    bytes_read += n;
  }

  if (send_block(clientfd, buffer, bytes_read) == EXIT_FAILURE) {
    perror("write output");
    return;
  }

  uint32_t end = 0;
  if (send_data(clientfd, &end, sizeof(end)) == EXIT_FAILURE) {
    perror("write end");
    return;
  }

  close(p[0]);
  wait(NULL);
}

/* Recebe o pedido do cliennte e valida o serviço pedido, e executa o servico */
void handle_client(int clientfd) {
  uint8_t service;

  if (receive_data(clientfd, &service, sizeof(service)) == EXIT_FAILURE) {
    perror("read service");
    return;
  }
  printf("Received service: %s\n", service == CPUINFO   ? "CPUINFO"
                                   : service == MEMINFO ? "MEMINFO"
                                                        : "UNKNOWN");
  if (service != CPUINFO && service != MEMINFO) {
    uint8_t status = INVALID_SERVICE;
    write(clientfd, &status, sizeof(status));
    send_status(clientfd, status);
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

  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind (inet)");

  return sockfd;
}

void client_protocol(int sockfd, uint8_t service){
  /* Envia o código do serviço */
  if (send_data(sockfd, &service, sizeof(service) == EXIT_FAILURE))
    fatal_system_error("write codigo do servico");

  uint8_t status;
  /* Recebe o estado da resposta */
  if (receive_data(sockfd, &status, sizeof(status)) == EXIT_FAILURE)
    fatal_system_error("read status do servidor");
  if (status == INVALID_SERVICE) {
    close(sockfd);
    fatal_system_error("Serviço inválido\n");
  }

  printf("received status: %s\n", status_to_string(status));

  uint32_t size;

  if (receive_data(sockfd, &size, sizeof(size)) == EXIT_FAILURE)
    fatal_system_error("read tamanho do bloco");

  if (size <= 0) {
    fprintf(stderr, "invalid size: %d\n", size);
    return;
  }

  char buffer[size+1];

  if (receive_data(sockfd, buffer, size) == EXIT_FAILURE)
    fatal_system_error("read dados");

  if (write(STDOUT_FILENO, buffer, size) == -1)
    fatal_system_error("write stdout");
}
