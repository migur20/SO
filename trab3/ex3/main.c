#include "countdown.h"
#include "threadpool.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DIM 1000000
#define N_TASKS 8
#define N_WORKERS 4
#define QUEUE_SIZE 16

typedef struct {
  long sum;
  short min;
  short max;
} Result;

typedef struct {
  short *values;
  unsigned long begin;
  unsigned long end;
  Result result;
  countdown_t *cd;
} TaskArgs;
///////////////////////////////////////////
// FUNÇÕES
///////////////////////////////////////////
short *vector_create(unsigned long dim)
{
  return malloc(dim * sizeof(short));
}

void vector_random_init(short values[], unsigned long dim)
{
  srand(2026);

  for (unsigned long i = 0; i < dim; ++i)
    values[i] = (short)(rand() % 1000);
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
Result sequential(short values[], unsigned long dim)
{
  Result r;

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
  TaskArgs *a = (TaskArgs *)arg;

  a->result = sequential(a->values + a->begin,
                         a->end - a->begin);

  countdown_down(a->cd);

  return NULL;
}
//////////////////////////////////////////////
// JUNTAR OS RESULTADOS PARCIAIS 
/////////////////////////////////////////////

