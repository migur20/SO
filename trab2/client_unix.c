#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"

/* Função principal do cliente */
int main(int argc, char *argv[])
{
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
    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

/* Envia o código do serviço */
    write(sockfd, &service, sizeof(service));

/* Recebe o estado da resposta */
    read(sockfd, &status, sizeof(status));

    if (status == INVALID_SERVICE) {
        printf("Serviço inválido\n");
        close(sockfd);
        return 1;
    }

// Recebe os blocos enviados pelo servidor
    while (1) {
        uint32_t size;
        char buffer[BUF_SIZE];

        read(sockfd, &size, sizeof(size));

        if (size == 0) {
            break;
        }

        while (size > 0) {
            int n = read(sockfd, buffer, BUF_SIZE);

            write(STDOUT_FILENO, buffer, n);

            size -= n;
        }
    }

    close(sockfd);

    return 0;
}