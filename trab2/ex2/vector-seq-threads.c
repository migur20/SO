#include <errno.h>
#include <error.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int verbose = 0;

typedef struct {
  long sum;
  int bigger;
  int smaller;
} ThreadReturn;

typedef struct {
  int id;
  short *v; // pointer posicao inicial
  unsigned long dim;
} ThreadArgs;

void fatal_system_error(const char *errorMsg) {
  perror(errorMsg);
  exit(EXIT_FAILURE);
}

void fatal_pthread_error(const char *errorMsg, int retvalue) {
  errno = retvalue;
  perror(errorMsg);
  exit(EXIT_FAILURE);
}

void check_pthread_error(int retvalue, const char *msg) {
  if (retvalue != 0)
    fatal_pthread_error(msg, retvalue);
}

// :declarations
void random_init();
long random_get_value(long min, long max);
short *vector_create_short(unsigned long dim);
void vector_init_short(short values[], unsigned long dim);
void vector_random_init_short(short values[], unsigned long dim);
void *thread_func(void *_args);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Not enough arguments\n");
    printf("Usage: ./vector-seq-threads <number_of_data> "
           "<number_of_threads> <optional:-v>\n");
    exit(EXIT_FAILURE);
  }
  if (argc > 3 && strcmp(argv[3], "-v") == 0) {
    verbose = 1;
  }

  int nthreads = 1;
  unsigned long dim;
  short *values;

  // Usage: ./vector-seq-threads <number_of_data> <number_of_threads>
  dim = atol(argv[1]);
  nthreads = atoi(argv[2]);

  random_init();

  values = NULL;
  values = vector_create_short(dim);
  if (values == NULL) {
    char buf[128];
    sprintf(buf, "Failed to allocate memory for %lu values\n", dim);
    fatal_system_error(buf);
  }

  printf("[MAIN]:Creating a vector of %lu (%.2f MB; %.2f GB) values and "
         "processing with %d threads\n",
         dim, dim / 1e6, dim / 1e9, nthreads);
  printf("[MAIN]:This will require approximately %.2f MB (%.2f GB) of "
         "memory\n",
         dim * sizeof(*values) / 1e6, dim * sizeof(*values) / 1e9);

  // vector_init_short(values, dim);
  vector_random_init_short(values, dim);

  int resto = dim % nthreads;
  dim = dim / nthreads;

  ThreadArgs args[nthreads];
  pthread_t th[nthreads];
  for (int i = 0; i < nthreads; i++) {
    args[i].v = &values[i * dim];
    args[i].dim = (i == nthreads - 1) ? dim + resto : dim;
    args[i].id = i + 1;
    check_pthread_error(pthread_create(&th[i], NULL, thread_func, &args[i]),
                        "Error creating thread");
  }

  long sum = 0;
  int bigger = values[0];
  int smaller = values[0];

  for (int i = 0; i < nthreads; i++) {
    ThreadReturn *ret;
    pthread_join(th[i], (void **)&ret);
    sum += ret->sum;
    if (ret->smaller < smaller)
      smaller = ret->smaller;
    if (ret->bigger > bigger)
      bigger = ret->bigger;
    free(ret);
  }

  printf("[MAIN]:smaller is %d\n", smaller);
  printf("[MAIN]:bigger  is %d\n", bigger);
  printf("[MAIN]:The sum is %ld\n", sum);

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

void *thread_func(void *_args) {
  ThreadArgs *args = (ThreadArgs *)_args;
  ThreadReturn *ret = calloc(1, sizeof(*ret)); // inicializa a 0
  *ret = (ThreadReturn){
      .sum = 0,
      .bigger = args->v[0],
      .smaller = args->v[0],
  };
  for (unsigned long j = 0; j < args->dim; ++j) {
    ret->sum += args->v[j];
    if (args->v[j] > ret->bigger)
      ret->bigger = args->v[j];
    if (args->v[j] < ret->smaller)
      ret->smaller = args->v[j];
  }
  if (verbose == 1) {
    printf("[THREAD[%d]]:smaller is %d\n", args->id, ret->smaller);
    printf("[THREAD[%d]]:bigger  is %d\n", args->id, ret->bigger);
    printf("[THREAD[%d]]:The sum is %ld\n", args->id, ret->sum);
  }
  return ret;
}
