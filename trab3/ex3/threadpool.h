#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "sharedbuffer_pthread_cond.h"
#include <stdbool.h>
#include <pthread.h>

#define END_WORK_ITEM (work_item_t*)NULL

/*
 * Function pointer type for work item operations.

typedef struct {
  void *(*operation)(void *);
  void *args;
} work_item_t;
*/

typedef void*(*function_t)(void *);

/**
 * Thread Pool structure
 */
typedef struct {
  SharedBuffer workQueue;
  pthread_t *workersThreads;
  int nWorkersThreads;
  bool shuttingDown;
  pthread_mutex_t mutex;
} threadpool_t;

int threadpool_init(threadpool_t *tp, int workQueueDim,
                    int numberWorkerThreads);

int threadpool_submit(threadpool_t *tp, function_t oper, void *args);

void threadpool_destroy(threadpool_t *tp);

void check_pthread_error(int retval, const char *msg);

#endif
