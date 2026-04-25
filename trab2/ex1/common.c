#include "common.h"

void fatal_system_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Envia um bloco no formato: [4 bytes dimensão][dados] */
void send_block(int fd, char *buffer, uint32_t size) {
  write(fd, &size, sizeof(size));
  write(fd, buffer, size);
}

/* Executa o serviço pedido: 1 -> lscpu; 2 -> free -h */
void run_service(int clientfd, uint8_t service) {
  int p[2];
  pipe(p);

  int pid = fork();

  if (pid == -1) {
    close(p[0]);
    close(p[1]);
    perror("forking");
    return;
  }

  /* Processo filho: redireciona o stdout para o pipe e executa o comando pedido
   */
  if (pid == 0) {

    close(p[0]);

    dup2(p[1], STDOUT_FILENO);
    close(p[1]);

    if (service == CPUINFO) {
      execlp("lscpu", "lscpu", NULL);
    } else {
      execlp("free", "free", "-h", NULL);
    }

    _exit(1);
  }
  /* Processo pai: lê o output do comando através do pipee envia esse output ao
   * cliente em blocos */
  close(p[1]);

  uint8_t status = OK;
  write(clientfd, &status, sizeof(status));

  char buffer[BUF_SIZE];
  int n;

  while ((n = read(p[0], buffer, BUF_SIZE)) > 0) {
    send_block(clientfd, buffer, n);
  }

  uint32_t end = 0;
  write(clientfd, &end, sizeof(end));

  close(p[0]);
  wait(NULL);
}

/* Recebe o pedido do cliente e valida o serviço pedido, e executa o servico */
void handle_client(int clientfd) {
  uint8_t service;

  read(clientfd, &service, sizeof(service));

  if (service != CPUINFO && service != MEMINFO) {
    uint8_t status = INVALID_SERVICE;
    write(clientfd, &status, sizeof(status));
    return;
  }

  run_service(clientfd, service);
}

/* Cria o socket UNIX do servidor e faz o bind ao pathname definido */
int create_unix_socket() {
  int sockfd;
  struct sockaddr_un addr;

  unlink(SOCKET_PATH);

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, SOCKET_PATH);

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    fatal_system_error("bind(unix)");

  return sockfd;
}

/* Cria o socket INET/TCP do servidor */
int create_inet_socket()
{
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
			fatal_system_error("bind (inet)");

    return sockfd;
}
