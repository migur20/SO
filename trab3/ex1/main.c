#include "countdown.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_DOWN 4
#define N_WAIT 3

countdown_t cd;

void *waiter(void *arg)
{
  int id = *(int *)arg;

  printf("Waiter %d vai esperar\n", id);

  int r = countdown_wait(&cd);

  printf("Waiter %d desbloqueou com retorno %d\n", id, r);

  return NULL;
}

void *worker(void *arg)
{
  int id = *(int *)arg;

  sleep(id + 1);

  printf("Worker %d fez countdown_down\n", id);

  countdown_down(&cd);

  return NULL;
}

int main(void)
{
  pthread_t waiters[N_WAIT];
  pthread_t workers[N_DOWN];

  int wait_ids[N_WAIT];
  int worker_ids[N_DOWN];

  countdown_init(&cd, N_DOWN);

  for (int i = 0; i < N_WAIT; ++i) {
    wait_ids[i] = i;
    pthread_create(&waiters[i], NULL, waiter, &wait_ids[i]);
  }

  for (int i = 0; i < N_DOWN; ++i) {
    worker_ids[i] = i;
    pthread_create(&workers[i], NULL, worker, &worker_ids[i]);
  }

  for (int i = 0; i < N_DOWN; ++i)
    pthread_join(workers[i], NULL);

  for (int i = 0; i < N_WAIT; ++i)
    pthread_join(waiters[i], NULL);

  printf("Teste wait depois de chegar a zero:\n");

  int r = countdown_wait(&cd);

  printf("Retorno final = %d\n", r);

  countdown_destroy(&cd);

  return EXIT_SUCCESS;
}