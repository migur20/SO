#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "sharedbuffer_pthread_cond.h"
#include <stdbool.h>

/*
 * Function pointer type for work item operations.
 */
typedef void*(*function_t)(void *);

/**
 * Thread Pool structure
 */
typedef struct {
  SharedBuffer workQueue;
  pthread_t *workersThreads;
  int nWorkersThreads;
} threadpool_t;

int threadpool_init(threadpool_t *tp, int workQueueDim,
                    int numberWorkerThreads);
int threadpool_submit(threadpool_t *tp, function_t oper, void *args);
void threadpool_destroy(threadpool_t *tp);

#endif
