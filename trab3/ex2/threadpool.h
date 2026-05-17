#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "sharedbuffer_pthread_cond.h"
#include <stdbool.h>
#include <pthread.h>

/*
 * Function pointer type for work item operations.
 */
typedef struct {
  void *(*operation)(void *);
  void *args;
} work_item_t;

typedef void*(*function_t)(void *);

/**
 * Thread Pool structure
 */
typedef struct {
  pthread_t *workersThreads;
  int nWorkersThreads;

  SharedBuffer workQueue;

  bool shuttingDown;
  pthread_mutex_t mutex;
} threadpool_t;

int threadpool_init(threadpool_t *tp, int nWorkerThreads,
                    int workQueueSize);
int threadpool_submit(threadpool_t *tp, work_item_t *wi);
void threadpool_destroy(threadpool_t *tp);

#endif
