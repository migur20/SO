#include <pthread.h>
#include <semaphore.h>

typedef struct {
  void **buffer;
  int iPut;
  int iGet;
  int nelems;
  int maxCapacity;
  pthread_cond_t cEsperaEspacoLivre;
  pthread_cond_t cEsperaEspacoOcupado;
  pthread_mutex_t mutex;
} SharedBuffer;

int sharedBuffer_init(SharedBuffer *sb, int capacity);
void sharedBuffer_destroy(SharedBuffer *sb);
void sharedBuffer_Put(SharedBuffer *sb, void *data);
void *sharedBuffer_Get(SharedBuffer *sb);
