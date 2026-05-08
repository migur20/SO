#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  // a definir com os atributos e mecanismos de sincronismo
  // necessários à sua implementação
  pthread_mutex_t mutex;
  sem_t sem;
  int count;
} countdown_t;

int countdown_init(countdown_t *cd, int initialValue)
{
  cd->count = initialValue;
  sem_init(&cd->sem, 0, 1);
  pthread_mutex_init(&cd->mutex, NULL);
  return EXIT_SUCCESS;
}

int countdown_destroy(countdown_t *cd)
{
  sem_destroy(&cd->sem);
  pthread_mutex_destroy(&cd->mutex);
  return EXIT_SUCCESS;
}

int countdown_wait(countdown_t *cd)
{
  if (cd->count == 0)
    return EXIT_FAILURE;
  sem_wait(&cd->sem);
  return EXIT_SUCCESS;
}

int countdown_down(countdown_t *cd)
{
  pthread_mutex_lock(&cd->mutex);
  {
    cd->count--;
    sem_post(&cd->sem);
  }
  pthread_mutex_unlock(&cd->mutex);

  return cd->count;
}

#define NVALUES 100
#define NTHREADS 10
#define DEC NVALUES/NTHREADS

void *func(void *_args)
{
  countdown_t *cd = (countdown_t *)_args;

	int c = 0;
  for (int i = 0; i < DEC; i++) {
    if (countdown_wait(cd) != EXIT_FAILURE){
      countdown_down(cd);
			sleep(1);
			printf("count: %d\n", cd->count);
			c++;
		}
  }
	printf("dec %d times\n", c);

  return NULL;
}

int main(void)
{
  countdown_t cd = {0};
  countdown_init(&cd, NVALUES);
  printf("count:%d\n", cd.count);

  pthread_t ths[NTHREADS];

  for (int i = 0; i < NTHREADS; i++) {
    pthread_create(&ths[i], 0, func, &cd);
  }

  for (int i = 0; i < NTHREADS; i++) {
    pthread_join(ths[i], NULL);
  }

  printf("count:%d\n", cd.count);
  countdown_destroy(&cd);
  return EXIT_SUCCESS;
}
