#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <pthread.h>

typedef struct {
  // a definir com os atributos e mecanismos de sincronismo
  // necessários à sua implementação
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
} countdown_t;

int countdown_init(countdown_t *cd, int initialValue);
int countdown_destroy(countdown_t *cd);
int countdown_wait(countdown_t *cd);
int countdown_down(countdown_t *cd);

#endif