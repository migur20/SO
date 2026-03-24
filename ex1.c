#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t retfork1, retfork2; 
    int status;


    // Primeiro filho: /bin/date
    retfork1 = fork();
    if (retfork1 < 0) { 
        perror("fork"); 
        exit(EXIT_FAILURE);
    }

    if (retfork1 == 0) {

        printf("[CHILD 1] PID=%d; PPID=%d - executando /bin/date\n", getpid(), getppid()); // Imprime o PID do filho e do pai
        execl("/bin/date", "date", NULL); 
        perror("execl date"); 
        exit(EXIT_FAILURE);
    }

    // Segundo filho: ping
    retfork2 = fork();
    if (retfork2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (retfork2 == 0) {

        printf("[CHILD 2] PID=%d; PPID=%d - executando ping\n", getpid(), getppid());  
        execl("/bin/ping", "ping", "-c", "4", "www.google.com", NULL);  
        perror("execl ping"); 
        exit(EXIT_FAILURE);
    }

    // Processo pai
    printf("[PARENT] PID=%d criou filhos %d e %d\n", getpid(), retfork1, retfork2); // Imprime os PIDs dos filhos
    printf("[PARENT] Esperando filhos terminarem...\n");
		fflush(stdout);

    // Espera pelo primeiro filho
    waitpid(retfork1, &status, 0); 
    printf("[PARENT] Filho %d terminou com status = %d\n", retfork1, WEXITSTATUS(status));

    // Espera pelo segundo filho
    waitpid(retfork2, &status, 0);
    printf("[PARENT] Filho %d terminou com status = %d\n", retfork2, WEXITSTATUS(status)); 

    printf("[PARENT] PID=%d terminando\n", getpid()); 
    return EXIT_SUCCESS;
}
