#include "common.h"

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr,
            "Uso: %s <num_clients> <ip_servidor> <port> <tamanho_data>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  char *args[] = {"./client", argv[2], argv[3], argv[4], NULL};

  int nclients = atoi(argv[1]);
  if (nclients <= 0)
    fatal_system_error("num clients invalido");

  for (int i = 0; i < nclients; i++) {
    pid_t retfork = fork();
    if (retfork < 0)
      fatal_system_error("fork");
    if (retfork == 0) {
      // CHILD
      execvp(args[0], args);
      fatal_system_error("exec");
    }
  }

  if (wait(NULL) == -1)
    fatal_system_error("wait");

  return EXIT_SUCCESS;
}
