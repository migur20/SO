#include "countdown.h"

#include <stdlib.h>

int countdown_init(countdown_t *cd, int initialValue)
{
  if (initialValue <= 0)
    return EXIT_FAILURE;

  cd->count = initialValue;

  pthread_mutex_init(&cd->mutex, NULL);
  pthread_cond_init(&cd->cond, NULL);

  return EXIT_SUCCESS;
}

int countdown_destroy(countdown_t *cd)
{
  pthread_mutex_destroy(&cd->mutex);
  pthread_cond_destroy(&cd->cond);

  return EXIT_SUCCESS;
}

int countdown_wait(countdown_t *cd)
{
  pthread_mutex_lock(&cd->mutex);

  if (cd->count == 0) {
    pthread_mutex_unlock(&cd->mutex);
    return EXIT_FAILURE;
  }

  while (cd->count > 0) {
    pthread_cond_wait(&cd->cond, &cd->mutex);
  }

  pthread_mutex_unlock(&cd->mutex);

  return EXIT_SUCCESS;
}

int countdown_down(countdown_t *cd)
{
  pthread_mutex_lock(&cd->mutex);

  if (cd->count > 0) {
    cd->count--;

    if (cd->count == 0) {
      pthread_cond_broadcast(&cd->cond);
    }
  }

  pthread_mutex_unlock(&cd->mutex);

  return EXIT_SUCCESS;
}