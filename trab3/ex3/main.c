// #include "countdown.h"
#include "threadpool.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void fatal_system_error(const char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

typedef struct {
  long sum;
  short min;
  short max;
} ThreadReturn;

typedef struct {
  short *values;
  unsigned long dim;
  ThreadReturn result;
  // countdown_t *cd;
} ThreadArgs;
///////////////////////////////////////////
// FUNÇÕES
///////////////////////////////////////////
///////////////////////////////////////////
void random_init()
{
  // Initialize the random number generator with the current time as the seed,
  // which ensures that we get a different sequence of random numbers each
  // time we run the program.
  // srandom(time(NULL));

  // Set a fixed seed for reproducibility, i.e. will generate the same sequence
  // of random numbers every time the program is run, which is useful for
  // debugging and testing
  srandom(2026);
}
long random_get_value(long min, long max)
{
  return min + random() % (max - min + 1);
}

short *vector_create_short(unsigned long dim)
{
  return malloc(dim * sizeof(short));
}

void vector_init_short(short values[], unsigned long dim)
{
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = i + 1;
  }
}

void vector_random_init_short(short values[], unsigned long dim)
{
  for (unsigned long i = 0; i < dim; ++i) {
    values[i] = random_get_value(SHRT_MIN, SHRT_MAX);
  }
}
///////////////////////////////////////////
// FUNÇÃO PARA MEDIR O TEMPO
///////////////////////////////////////////
double get_time(void)
{
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);

  return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}
///////////////////////////////////////////
// FUNÇÃO SEQUENCIAL
///////////////////////////////////////////
ThreadReturn sequential(short values[], unsigned long dim)
{
  ThreadReturn r;

  r.sum = 0;
  r.min = values[0];
  r.max = values[0];

  for (unsigned long i = 0; i < dim; ++i) {
    short v = values[i];

    r.sum += v;

    if (v < r.min)
      r.min = v;

    if (v > r.max)
      r.max = v;
  }

  return r;
}
//////////////////////////////////////////////
// TAREFA DO THREAD POOL
/////////////////////////////////////////////
void *pool_task(void *arg)
{
  ThreadArgs *a = (ThreadArgs *)arg;

  a->result = sequential(a->values, a->dim);

  // countdown_down(a->cd);

  return NULL;
}
//////////////////////////////////////////////
// JUNTAR OS RESULTADOS PARCIAIS
/////////////////////////////////////////////

#define N_TASKS 8
#define N_WORKERS 4
#define QUEUE_SIZE 16

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "usage: %s <dim> <number-of-worker-threads>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int dim = atoi(argv[1]);
  int nthreads = atoi(argv[2]);

  short *values = vector_create_short(dim);
	random_init();
  vector_random_init_short(values, dim);
  // for (int i = 0; i < dim; i++)
  //   printf("%d, ", values[i]);
  // printf("\n");

  int resto = dim % nthreads;
  dim = dim / nthreads;

  threadpool_t tp = {0};
  if (threadpool_init(&tp, QUEUE_SIZE, nthreads) == -1)
    fatal_system_error("threadpool init");

  ThreadArgs args[nthreads];
  for (int i = 0; i < nthreads; i++) {
    args[i].values = &values[i * dim];
    args[i].dim = (i == nthreads - 1) ? dim + resto : dim;
    threadpool_submit(&tp, pool_task, &args[i]);
  }

  threadpool_destroy(&tp);

  long sum = 0;
  int bigger = values[0];
  int smaller = values[0];

  for (int i = 0; i < nthreads; i++) {
    ThreadReturn ret = args[i].result;
    sum += ret.sum;
    if (ret.min < smaller)
      smaller = ret.min;
    if (ret.max > bigger)
      bigger = ret.max;
    printf("min:%d\nmax:%d\nsum:%ld\n---------------\n", ret.min, ret.max,
           ret.sum);
  }

  printf("smaller: %d, bigger: %d, sum: %ld\n", smaller, bigger, sum);

  return EXIT_SUCCESS;
}
