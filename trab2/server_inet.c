#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "common.h"

/* Cria o socket INET/TCP do servidor */
int create_inet_socket()
{
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    return sockfd;
}

/* Envia um bloco: [dimensão][dados] */
void send_block(int fd, char *buffer, uint32_t size)
{
    write(fd, &size, sizeof(size));
    write(fd, buffer, size);
}

/* Executa lscpu ou free -h */
void run_service(int clientfd, uint8_t service)
{
    int p[2];
    char buffer[BUF_SIZE];
    int n;

    pipe(p);

    if (fork() == 0) {
        close(p[0]);

        dup2(p[1], STDOUT_FILENO);
        close(p[1]);

        if (service == CPUINFO)
            execlp("lscpu", "lscpu", NULL);
        else
            execlp("free", "free", "-h", NULL);

        _exit(1);
    }

    close(p[1]);

    uint8_t status = OK;
    write(clientfd, &status, sizeof(status));

    while ((n = read(p[0], buffer, BUF_SIZE)) > 0) {
        send_block(clientfd, buffer, n);
    }

    uint32_t end = 0;
    write(clientfd, &end, sizeof(end));

    close(p[0]);
    wait(NULL);
}

/* Lê o pedido do cliente e trata o serviço */
void handle_client(int clientfd)
{
    uint8_t service;

    read(clientfd, &service, sizeof(service));

    if (service != CPUINFO && service != MEMINFO) {
        uint8_t status = INVALID_SERVICE;
        write(clientfd, &status, sizeof(status));
        return;
    }

    run_service(clientfd, service);
}

/* Servidor principal */
int main()
{
    int sock_inet;
    int clientfd;

    sock_inet = create_inet_socket();

    listen(sock_inet, 5);

    while (1) {
        clientfd = accept(sock_inet, NULL, NULL);

        handle_client(clientfd);

        close(clientfd);
    }

    close(sock_inet);

    return 0;
}