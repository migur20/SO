#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void fatal_system_error(const char *errorMsg) {
  perror(errorMsg);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
		printf("Not enough arguments\n");
		exit(EXIT_FAILURE);
  }

  // Usage: ./vector-seq-processes <number_of_data> <number_of_processes>
  long max = atol(argv[1]);
  int nProcesses = atoi(argv[2]);

  // Quantidade de dados em cada processo
  max = max / nProcesses;

	char max_str[32];
  sprintf(max_str, "%ld", max); //Argumento com o tamanho do vector
	char *args[] = {"./vector-seq", max_str, NULL};

  for (int i = 0; i < nProcesses; i++) {
    pid_t retfork = fork();

    if (retfork < 0)
      fatal_system_error("vector-seq-processes error forking");
    if (retfork == 0) {
      // Child
			//execvp(args[0], args);
			execvp(args[0], args);
      fatal_system_error("vector-seq-processes child process returned");
    }
  }

	wait(NULL);

  return EXIT_SUCCESS;
}
