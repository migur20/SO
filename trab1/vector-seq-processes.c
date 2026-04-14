#include <error.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void fatal_system_error(const char *errorMsg) {
  perror(errorMsg);
  exit(EXIT_FAILURE);
}

// :declarations
void random_init();
long random_get_value(long min, long max);
short *vector_create_short(unsigned long dim);
void vector_init_short(short values[], unsigned long dim);
void vector_random_init_short(short values[], unsigned long dim);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Not enough arguments\n");
    printf("Usage: ./vector-seq-processes <number_of_data> "
           "<number_of_processes>\n");
    exit(EXIT_FAILURE);
  }

  // Usage: ./vector-seq-processes <number_of_data> <number_of_processes>
  unsigned long max = atol(argv[1]);
  int nProcesses = atoi(argv[2]);

  random_init();

  short *values = NULL;
  values = vector_create_short(max);
  if (values == NULL) {
    char buf[128];
    sprintf(buf, "Failed to allocate memory for %lu values\n", max);
    fatal_system_error(buf);
    exit(EXIT_FAILURE);
  }
  // vector_init_short(values, max);
  vector_random_init_short(values, max);

  printf("PARENT:Creating a vector of %lu (%.2f MB; %.2f GB) values\n", max,
         max / 1e6, max / 1e9);
  printf("PARENT:This will require approximately %.2f MB (%.2f GB) of "
         "memory\n",
         max * sizeof(*values) / 1e6, max * sizeof(*values) / 1e9);

  int resto = max % nProcesses;
  max = max / nProcesses;

  int start_idx[nProcesses];
  for (int i = 0; i < nProcesses; i++) {
    start_idx[i] = i * max;
  }
  // Quantidade de dados em cada processo

  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    fatal_system_error("error creating pipe");
  }

  for (int i = 0; i < nProcesses; i++) {
    pid_t retfork = fork();
    if (retfork < 0)
      fatal_system_error("vector-seq-processes error forking");

    if (retfork == 0) {
      // :child
      if (i == nProcesses - 1) {
        max += resto;
      }

      long sum = 0;
      int bigger = values[start_idx[i]];
      int smaller = values[start_idx[i]];
      for (unsigned long j = start_idx[i]; j < start_idx[i] + max; ++j) {
        sum += values[j];
        if (values[j] > bigger)
          bigger = values[j];
        if (values[j] < smaller)
          smaller = values[j];
      }
      printf("CHILD[%d]:smaller is %d\n", getpid(), smaller);
      printf("CHILD[%d]:bigger  is %d\n", getpid(), bigger);
      printf("CHILD[%d]:The sum is %ld\n", getpid(), sum);

      // child doesnt need to read from the pipe
      close(pipe_fd[0]);

      char buf[128];
      sprintf(buf, "%ld,%d,%d", sum, bigger, smaller); // write on buffer
      write(pipe_fd[1], buf, sizeof(buf)); // write buffer to the pipe

      close(pipe_fd[1]);

      exit(EXIT_SUCCESS);
    }
  }
  // :parent

  // parent doesnt need to write on the pipe
  close(pipe_fd[1]);

  printf("PARENT:Waiting for childs to finnish\n");

  while (wait(NULL) > 0) {
    ;
  }

  long sum = 0;
  int bigger = 0;
  int smaller = 0;

  char buf[128];
  int bytes_read;
  while ((bytes_read = read(pipe_fd[0], buf, 128)) != 0) {
    if (bytes_read < 0) {
      fatal_system_error("Error reading from pipe");
    }
    long s;
    int big;
    int small;
    sscanf(buf, "%ld,%d,%d", &s, &big, &small);
    sum += s;
    if (bigger < big)
      bigger = big;
    if (smaller > small)
      smaller = small;
  }
  close(pipe_fd[0]);

  printf("PARENT:smaller is %d\n", smaller);
  printf("PARENT:bigger  is %d\n", bigger);
  printf("PARENT:The sum is %ld\n", sum);

  free(values);

  return EXIT_SUCCESS;
}

void random_init() {
  // Initialize the random number generator with the current time as the seed,
  // which ensures that we get a different sequence of random numbers each
  // time we run the program.
  // srandom(time(NULL));

  // Set a fixed seed for reproducibility, i.e. will generate the same sequence
  // of random numbers every time the program is run, which is useful for
  // debugging and testing
  srandom(2026);
}

long random_get_value(long min, long max) {
  return min + random() % (max - min + 1);
}

//
// vector functions for short type
//
short *vector_create_short(unsigned long dim) {
  return malloc(dim * sizeof(short));
}

void vector_init_short(short values[], unsigned long dim) {
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = i + 1;
  }
}

void vector_random_init_short(short values[], unsigned long dim) {
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = random_get_value(SHRT_MIN, SHRT_MAX);
  }
}
