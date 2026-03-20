#include <sys/types.h>
#include <sys/wait.h>
#define _XOPEN_SOURCE 600 // for clock_gettime
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

void fatal_system_error(const char *errorMsg) {
  perror(errorMsg);
  exit(EXIT_FAILURE);
}

int main(void) {
  struct timespec t_start;
  struct timespec t_end;
  // Get the start time
  clock_gettime(CLOCK_MONOTONIC, &t_start);
  // code to be timed goes here

  pid_t retfork = fork();
  if (retfork < 0) {
		fatal_system_error("ex3, error forking");
  }

	if(retfork == 0){
		//Child
		char *args[] = {"./vector-seq-processes", "1000000000", "2", NULL};
		execvp(args[0], args);
		fatal_system_error("ex3, error executing");
	}

	waitpid(retfork, NULL, 0);

  // Get the end time
  clock_gettime(CLOCK_MONOTONIC, &t_end);
  // Calculate the elapsed time in seconds
  double elapsed_time = (t_end.tv_sec - t_start.tv_sec) +
                        (t_end.tv_nsec - t_start.tv_nsec) * 1e-9;
  // Print the elapsed time in seconds and microseconds
  printf("Total elapsed time = %9.6lfs (%9.6lfus)\n", elapsed_time,
         elapsed_time * 1e6);
  return 0;
}
