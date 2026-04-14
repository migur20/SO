#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVICE_CPUINFO 1
#define SERVICE_MEMINFO 2

int main(int argc, char *argv[]) {
    int fd;
    struct sockaddr_in addr;
    uint8_t service, status;
    int port;

    // verificar argumentos
    if (argc != 4) {
        printf("Uso: %s <IP> <PORTO> <CPUINFO|MEMINFO>\n", argv[0]);
        return 1;
    }

    // escolher serviço
    if (strcmp(argv[3], "CPUINFO") == 0)
        service = SERVICE_CPUINFO;
    else if (strcmp(argv[3], "MEMINFO") == 0)
        service = SERVICE_MEMINFO;
    else {
        printf("Serviço inválido\n");
        return 1;
    }

    port = atoi(argv[2]);

    // criar socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // configurar endereço do servidor
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // converter IP para formato binário
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(fd);
        return 1;
    }

    // ligar ao servidor
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    // enviar pedido (1 byte)
    write(fd, &service, 1);

    // receber status
    read(fd, &status, 1);

    if (status != 0) {
        printf("Erro do servidor\n");
        close(fd);
        return 1;
    }

    // receber blocos de dados
    while (1) {
        uint32_t len;
        char buf[4096];

        read(fd, &len, 4);
        len = ntohl(len);

        if (len == 0) break; // fim

        read(fd, buf, len);
        write(1, buf, len); // imprimir no terminal
    }

    close(fd);
    return 0;
}