#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/sot_srv.sock"

#define SERVICE_CPUINFO 1
#define SERVICE_MEMINFO 2

#define STATUS_OK 0
#define STATUS_INVALID_SERVICE 1
#define STATUS_EXEC_ERROR 2

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_un addr;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("Servidor à escuta em %s\n", SOCKET_PATH);

    while (1) {
        uint8_t service, status;
        int pipefd[2];
        pid_t pid;
        char buffer[4096];
        ssize_t n;
        int child_status;

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        if (read(client_fd, &service, 1) != 1) {
            close(client_fd);
            continue;
        }

        if (service != SERVICE_CPUINFO && service != SERVICE_MEMINFO) {
            status = STATUS_INVALID_SERVICE;
            write(client_fd, &status, 1);
            close(client_fd);
            continue;
        }

        if (pipe(pipefd) < 0) {
            status = STATUS_EXEC_ERROR;
            write(client_fd, &status, 1);
            close(client_fd);
            continue;
        }

        pid = fork();
        if (pid < 0) {
            status = STATUS_EXEC_ERROR;
            write(client_fd, &status, 1);
            close(pipefd[0]);
            close(pipefd[1]);
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            if (service == SERVICE_CPUINFO)
                execlp("lscpu", "lscpu", (char *)NULL);
            else
                execlp("free", "free", "-h", (char *)NULL);

            _exit(1);
        }

        close(pipefd[1]);

        status = STATUS_OK;
        write(client_fd, &status, 1);

        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            uint32_t len = htonl((uint32_t)n);
            write(client_fd, &len, 4);
            write(client_fd, buffer, n);
        }

        close(pipefd[0]);
        waitpid(pid, &child_status, 0);

        uint32_t end = 0;
        write(client_fd, &end, 4);

        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}