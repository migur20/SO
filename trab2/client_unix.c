#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/sot_srv.sock"

#define SERVICE_CPUINFO 1
#define SERVICE_MEMINFO 2

int main(int argc, char *argv[]) {
    int fd;
    struct sockaddr_un addr;
    uint8_t service, status;

    if (argc != 2) {
        printf("Uso: %s <CPUINFO|MEMINFO>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "CPUINFO") == 0)
        service = SERVICE_CPUINFO;
    else if (strcmp(argv[1], "MEMINFO") == 0)
        service = SERVICE_MEMINFO;
    else {
        printf("Serviço inválido\n");
        return 1;
    }

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    write(fd, &service, 1);
    read(fd, &status, 1);

    if (status != 0) {
        printf("Erro do servidor\n");
        close(fd);
        return 1;
    }

    while (1) {
        uint32_t len;
        char buffer[4096];

        read(fd, &len, 4);
        len = ntohl(len);

        if (len == 0) break;

        read(fd, buffer, len);
        write(STDOUT_FILENO, buffer, len);
    }

    close(fd);
    return 0;
}