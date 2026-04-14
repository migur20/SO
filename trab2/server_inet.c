#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 5000   // porto onde o servidor fica à escuta

#define SERVICE_CPUINFO 1
#define SERVICE_MEMINFO 2

#define STATUS_OK 0
#define STATUS_INVALID_SERVICE 1
#define STATUS_EXEC_ERROR 2

int main(void) {
    int sfd, cfd;
    struct sockaddr_in addr;

    // criar socket TCP (domínio Internet)
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        return 1;
    }

    // configurar endereço (IP + porto)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);           // converter porto para formato de rede
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // aceita ligações de qualquer IP

    // associar socket ao endereço
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // colocar socket à escuta
    if (listen(sfd, 5) < 0) {
        perror("listen");
        return 1;
    }

    printf("Servidor INET à escuta no porto %d\n", PORT);

    while (1) {
        uint8_t service, status;
        int p[2];           // pipe para capturar output do comando
        pid_t pid;
        char buf[4096];
        ssize_t n;

        // aceitar ligação de um cliente
        cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;

        // ler 1 byte com o código do serviço
        if (read(cfd, &service, 1) != 1) {
            close(cfd);
            continue;
        }

        // verificar se o serviço é válido
        if (service != SERVICE_CPUINFO && service != SERVICE_MEMINFO) {
            status = STATUS_INVALID_SERVICE;
            write(cfd, &status, 1);
            close(cfd);
            continue;
        }

        // criar pipe para capturar stdout do comando
        if (pipe(p) < 0) {
            status = STATUS_EXEC_ERROR;
            write(cfd, &status, 1);
            close(cfd);
            continue;
        }

        // criar processo filho
        pid = fork();
        if (pid < 0) {
            status = STATUS_EXEC_ERROR;
            write(cfd, &status, 1);
            close(p[0]);
            close(p[1]);
            close(cfd);
            continue;
        }

        if (pid == 0) {
            // filho

            close(p[0]);              // não lê do pipe
            dup2(p[1], 1);            // redireciona stdout para o pipe
            close(p[1]);

            // executar comando
            if (service == SERVICE_CPUINFO)
                execlp("lscpu", "lscpu", (char *)NULL);
            else
                execlp("free", "free", "-h", (char *)NULL);

            _exit(1); // erro no exec
        }

        // pai
        close(p[1]); // não escreve no pipe

        // enviar status OK
        status = STATUS_OK;
        write(cfd, &status, 1);

        // enviar output em blocos
        while ((n = read(p[0], buf, sizeof(buf))) > 0) {
            uint32_t len = htonl((uint32_t)n); // converter tamanho
            write(cfd, &len, 4);
            write(cfd, buf, n);
        }

        close(p[0]);
        wait(NULL); // esperar pelo filho

        // enviar bloco final (tamanho 0)
        uint32_t end = 0;
        write(cfd, &end, 4);

        close(cfd);
    }
}