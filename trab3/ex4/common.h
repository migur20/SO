#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#define SOCKET_PATH "/tmp/socket_so_tp2"
#define BUF_SIZE 1048

#define OK (uint8_t)0
#define INVALID_SERVICE (uint8_t)1
#define EXEC_ERROR (uint8_t)2
#define status_to_string(status)                                               \
  (status == OK)                ? "OK"                                         \
  : (status == INVALID_SERVICE) ? "INVALID_SERVICE"                            \
  : (status == EXEC_ERROR)      ? "EXEC_ERROR"                                 \
                                : "UNKNOWN"

// error management
void fatal_system_error(const char *msg);

// server management
/* Recebe size bytes de dados e coloca-os em buffer */
int receive_data(int fd, void *buffer, size_t size);
/* Envia size bytes de dados de buffer */
int send_data(int fd, void *buffer, size_t size);
/* Envia um bloco no formato: [4 bytes dimensão][dados] */
int send_block(int fd, void *buffer, uint32_t size);
/* Envia um status de servico e a mensagem de erro em caso de erro*/
int send_status(int clientfd, uint8_t status, char *msg);

// sockets creation
/* Cria o socket INET/TCP do servidor */
int create_inet_socket(int port);
int create_unix_socket();

#endif
