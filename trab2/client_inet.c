#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

/* Cliente INET/TCP */
int main(int argc, char *argv[])
{
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
    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    /* Envia o código do serviço */
    write(sockfd, &service, sizeof(service));

    /* Recebe o status */
    read(sockfd, &status, sizeof(status));

    if (status == INVALID_SERVICE) {
        printf("Serviço inválido\n");
        close(sockfd);
        return 1;
    }

    /* Recebe e mostra os blocos enviados pelo servidor */
    while (1) {
        uint32_t size;
        char buffer[BUF_SIZE];

        read(sockfd, &size, sizeof(size));

        if (size == 0)
            break;

        while (size > 0) {
            int n = read(sockfd, buffer, BUF_SIZE);

            write(STDOUT_FILENO, buffer, n);

            size -= n;
        }
    }

    close(sockfd);

    return 0;
}