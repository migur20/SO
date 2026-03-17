#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t retfork1, retfork2;
    int status;

    // criar primeiro filho 
    retfork1 = fork();
    if (retfork1 < 0 ) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (retfork1 == 0) {
        // Child 1 process
        printf("[SON 1] PID=%d executing /bin/date\n", getpid());

        execl("/bin/date", "date", NULL);

        perror("execl date");
        exit(EXIT_FAILURE);
    }

    // Criar segundo filho
    retfork2 = fork();
    if (retfork2 < 0 ) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (retfork2 == 0) {
        // Child 2 process
        printf("[SON 2] PID=%d executing ping\n", getpid());

        execl("/bin/ping", "ping", "-c", "4", "www.google.com", NULL);

        perror("execl ping");
        exit(EXIT_FAILURE);
    }

    // Parent process
    printf("[FATHER] PID=%d created children %d and %d\n", getpid(), retfork1, retfork2);
    printf("[FATHER] Waiting for children to finish...\n");

    // Wait for first child
    waitpid(retfork1, &status, 0);
    printf("[FATHER] Child %d finished with status = %d\n", retfork1, WEXITSTATUS(status));

    // Wait for second child
    waitpid(retfork2, &status, 0);
    printf("[FATHER] Child %d finished with status = %d\n", retfork2, WEXITSTATUS(status));

    return EXIT_SUCCESS;
}
