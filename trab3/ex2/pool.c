#include "threadpool.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_WORKERS 4
#define N_JOBS 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void *job(void *arg)
{
  int id = *(int *)arg;

  usleep(100000);

  pthread_mutex_lock(&mutex);
  counter++;
  printf("Job %d executado. Counter = %d\n", id, counter);
  pthread_mutex_unlock(&mutex);

  free(arg);

  return NULL;
}

int main()
{
  threadpool_t tp;

  if (threadpool_init(&tp, 8, N_WORKERS) < 0) {
    printf("Erro no threadpool_init\n");
    return 1;
  }

  for (int i = 0; i < N_JOBS; ++i) {
    int *id = malloc(sizeof(int));
    *id = i;

    if (threadpool_submit(&tp, job, id) < 0) {
      printf("Erro no submit\n");
      free(id);
    }
  }

  threadpool_destroy(&tp);

  printf("Resultado final: %d/%d jobs executados\n", counter, N_JOBS);

  if (counter == N_JOBS)
    printf("TESTE OK\n");
  else
    printf("TESTE FALHOU\n");

  return 0;
}