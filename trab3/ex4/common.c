#include "common.h"
#include <stdint.h>


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

int send_status(int clientfd, uint8_t status, char *msg) {
  if (send_data(clientfd, &status, sizeof(status)) == EXIT_FAILURE)
    return EXIT_FAILURE;
  if (status != OK)
    return send_block(clientfd, msg, strlen(msg));
	return EXIT_SUCCESS;
}

/* Cria o socket INET/TCP do servidor */
int create_inet_socket(int port) {
  int sockfd;
  struct sockaddr_in addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind");

  return sockfd;
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
