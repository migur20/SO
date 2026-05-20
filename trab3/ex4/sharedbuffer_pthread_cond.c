#include "sharedbuffer_pthread_cond.h"
#include <stdlib.h>

int sharedBuffer_init(SharedBuffer *sb, int capacity)
{
  sb->buffer = (void **)malloc(capacity * sizeof(void *));
  if (sb->buffer == NULL)
    return -1;
  sb->iGet = 0;
  sb->iPut = 0;
  sb->nelems = 0;
  sb->maxCapacity = capacity;
	int ret = 0;
  ret += pthread_cond_init(&sb->cEsperaEspacoLivre, NULL);
  ret += pthread_cond_init(&sb->cEsperaEspacoOcupado, NULL);
  ret += pthread_mutex_init(&sb->mutex, NULL);
	return ret;
}

void sharedBuffer_destroy(SharedBuffer *sb)
{
  free(sb->buffer);
  pthread_cond_destroy(&sb->cEsperaEspacoLivre);
  pthread_cond_destroy(&sb->cEsperaEspacoOcupado);
  pthread_mutex_destroy(&sb->mutex);
}

void sharedBuffer_Put(SharedBuffer *sb, void *data)
{
  pthread_mutex_lock(&sb->mutex);
  while (sb->nelems == sb->maxCapacity) {
    pthread_cond_wait(&sb->cEsperaEspacoLivre, &sb->mutex);
  }
  sb->buffer[sb->iPut] = data;
  sb->iPut = (sb->iPut + 1) % sb->maxCapacity;
  ++sb->nelems;
  pthread_cond_signal(&sb->cEsperaEspacoOcupado);
  pthread_mutex_unlock(&sb->mutex);
}

void *sharedBuffer_Get(SharedBuffer *sb)
{
  void *ret;
  pthread_mutex_lock(&sb->mutex);
  while (sb->nelems == 0) {
    pthread_cond_wait(&sb->cEsperaEspacoOcupado, &sb->mutex);
  }
  ret = sb->buffer[sb->iGet];
  sb->iGet = (sb->iGet + 1) % sb->maxCapacity;
  --sb->nelems;
  pthread_cond_signal(&sb->cEsperaEspacoLivre);
  pthread_mutex_unlock(&sb->mutex);
  return ret;
}
