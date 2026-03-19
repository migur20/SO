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

    if (argc != 2) { //programa deve receber 2 args
        fprintf(stderr, "Uso: %s <ficheiro>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd[2]; // Array para guardar os descritores do pipe

    if (pipe(fd) == -1) //se falhar retorna -1
        fatal_system_error("Erro no pipe");

    pid_t pid = fork(); //cria um novo processo, processo filho

    if (pid < 0) //erro ao criar processo, não sei se necessário 
        fatal_system_error("Erro no fork");

    if (pid == 0) { //processo filho 

        close(fd[0]); //O filho não vai ler do pipe. Então fechamos a leitura

        if (dup2(fd[1], STDOUT_FILENO) == -1) //Redireciona o stdout para o pipe, tudo o que o filho imprimir vai para o pipe
            fatal_system_error("Erro no dup2");

        close(fd[1]);

        execlp("wc", "wc", "-w", argv[1], (char *)NULL); //substitui o processo atual, o filho passa a ser o programa wc

        fatal_system_error("Erro ao executar wc");
    }

    close(fd[1]); // o pai nao escreve no pipe, fecha a escrita

    char buffer[128];
    ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1); //Lê dados do pipe

    if (n < 0)
        fatal_system_error("Erro na leitura do pipe");

    buffer[n] = '\0';

    close(fd[0]); //Fecha o pipe

    int status;

    if (waitpid(pid, &status, 0) == -1) // o pai espera o processo filho terminar. Evita processos zombie
        fatal_system_error("Erro no waitpid");

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) //verifica se o wc terminou corretamente
        fatal_system_error("Erro: o comando wc terminou com erro.\n");

    long palavras;

    if (sscanf(buffer, "%ld", &palavras) != 1)
        fatal_system_error("Erro: nao foi possivel obter o numero de palavras.\n");

    printf("%ld\n", palavras);

    return EXIT_SUCCESS;
}
