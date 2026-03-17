#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void fatal_system_error(const char *errorMsg) {
    perror(errorMsg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd[2];

    if (pipe(fd) == -1)
        fatal_system_error("pipe");

    pid_t pid = fork();

    if (pid < 0)
        fatal_system_error("fork");

    if (pid == 0) {

        close(fd[0]); // filho não lê

        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        execlp("wc", "wc", "-w", argv[1], NULL);

        fatal_system_error("exec");
    }

    close(fd[1]); // pai não escreve

    char buffer[128];
    read(fd[0], buffer, sizeof(buffer));

    printf("Number of words: %s", buffer);

    close(fd[0]);

    waitpid(pid, NULL, 0);

    return 0;
}