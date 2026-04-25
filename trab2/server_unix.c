#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "common.h"

/* Cria o socket UNIX do servidor e faz o bind ao pathname definido */
int create_unix_socket()
{
    int sockfd;
    struct sockaddr_un addr;

    unlink(SOCKET_PATH);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    return sockfd;
}

/* Envia um bloco no formato: [4 bytes dimensão][dados] */
void send_block(int fd, char *buffer, uint32_t size)
{
    write(fd, &size, sizeof(size));
    write(fd, buffer, size);
}

/* Executa o serviço pedido: 1 -> lscpu; 2 -> free -h */
void run_service(int clientfd, uint8_t service)
{
    int p[2];
    pipe(p);

    int pid = fork();

/* Processo filho: redireciona o stdout para o pipe e executa o comando pedido */
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
/* Processo pai: lê o output do comando através do pipee envia esse output ao cliente em blocos */
    close(p[1]);

    uint8_t status = OK;
    write(clientfd, &status, sizeof(status));

    char buffer[BUF_SIZE];
    int n;

    while ((n = read(p[0], buffer, BUF_SIZE)) > 0) {
        send_block(clientfd, buffer, n);
    }

    uint32_t end = 0;
    write(clientfd, &end, sizeof(end));

    close(p[0]);
    wait(NULL);
}

/* Recebe o pedido do cliente e valida o serviço pedido */
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

/* Função principal do servidor */
int main()
{
    int sock_unix;

    sock_unix = create_unix_socket();

    listen(sock_unix, 5);

    while (1) {
        int clientfd = accept(sock_unix, NULL, NULL);

        handle_client(clientfd);

        close(clientfd);
    }

    close(sock_unix);
    unlink(SOCKET_PATH);

    return 0;
}