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

#define SOCKET_PATH "/tmp/socket_so_tp2"
#define BUF_SIZE 1048
#define SERVER_PORT 5000

#define CPUINFO 1
#define MEMINFO 2

#define OK 0
#define INVALID_SERVICE 1
#define EXEC_ERROR 2

#define status_to_string(status)                                               \
  (status == OK)                ? "OK"                                         \
  : (status == INVALID_SERVICE) ? "INVALID_SERVICE"                            \
  : (status == EXEC_ERROR)      ? "EXEC_ERROR"                                 \
                                : "UNKNOWN"

// error management
void fatal_system_error(const char *msg);

// server management
/* Envia um bloco no formato: [4 bytes dimensão][dados] */
int send_block(int fd, void *buffer, uint32_t size);
/* Executa o serviço pedido: 1 -> lscpu; 2 -> free -h */
void run_service(int clientfd, uint8_t service);
/* Recebe o pedido do cliente, valida o serviço pedido, e executa o servico */
void handle_client(int clientfd);

int receive_data(int fd, void *buffer, size_t size);
int send_data(int fd, void *buffer, size_t size);

// sockets creation
/* Cria o socket INET/TCP do servidor */
int create_inet_socket();
/* Cria o socket UNIX do servidor e faz o bind ao pathname definido */
int create_unix_socket();

void client_protocol(int sockfd, uint8_t service);

#endif
