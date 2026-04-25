#include "common.h"

int main() {

    int sock_unix, sock_inet;
    int pid;

    //criar os sockets
    sock_unix = create_unix_socket();
    sock_inet = create_inet_socket();

    listen(sock_unix, 5);
    listen(sock_inet, 5);

    pid = fork();

    // FILHO → UNIX
    if (pid == 0) {
        close(sock_inet);  //porque filho nao precisa de INET 

        int clientfd;

        while (1) {
            clientfd = accept(sock_unix, NULL, NULL);
            handle_client(clientfd);
            close(clientfd);
        }

    // PAI → INET
    } else {
        close(sock_unix);  

        int clientfd;
        
        while (1) {
            clientfd = accept(sock_inet, NULL, NULL);
            handle_client(clientfd);
            close(clientfd);
        }
    }

    return 0;
}
