#include "threadpool.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void fatal_pthread_error(int retval, const char *msg)
{
  errno = retval;
  perror(msg);
  exit(EXIT_FAILURE);
}

void check_pthread_error(int retval, const char *msg)
{
  if (retval != 0) {
    fatal_pthread_error(retval, msg);
  }
}
/**
 * struct is used to represent a work item and is internal to the thread pool
 * implementation. Work item structure. Work items instances will be submit to
 * thread pool.
 */
typedef struct {
  function_t operation;
  void *args;
  bool isdynamicAlloc; // indicates if the work item was dynamically allocated
  // (using workitem_new) or not (using workitem_init)
} work_item_t;

void workitem_init(work_item_t *wt, function_t oper, void *args)
{
  wt->operation = oper;
  wt->args = args;
  wt->isdynamicAlloc = false;
}

work_item_t *workitem_new(function_t oper, void *args)
{
  work_item_t *wt = malloc(sizeof(work_item_t));
  if (wt == NULL)
    return NULL;

  workitem_init(wt, oper, args);
  wt->isdynamicAlloc = true;

  return wt;
}

void workitem_destroy(work_item_t *wt)
{
  if (wt->isdynamicAlloc)
    free(wt);
}

/**
 * Works threads code
 * param args
 * return void*
 */
void *threadpool_work_thread(void *args)
{
  threadpool_t *tp = (threadpool_t *)args;
  while (true) {
    work_item_t *wi = sharedBuffer_Get(&tp->workQueue);
    if (wi == END_WORK_ITEM)
      break;
    wi->operation(wi->args);
    workitem_destroy(wi);
  }
  return NULL;
}

/*
 * Initialize the thread pool
 * tp Pointer to the thread pool structure
 * workQueueDim Dimension of the work queue
 * numberWorkerThreads Number of worker threads
 * return 0 on success, -1 on failure
 */
int threadpool_init(threadpool_t *tp, int workQueueDim,
                    int numberWorkerThreads)
{
  if (sharedBuffer_init(&tp->workQueue, workQueueDim) < 0)
    return -1;

  tp->workersThreads = malloc(numberWorkerThreads * sizeof(pthread_t));
  if (tp->workersThreads == NULL) {
    sharedBuffer_destroy(&tp->workQueue);
    return -1;
  }

  tp->nWorkersThreads = numberWorkerThreads;
  tp->shuttingDown = false;

  int retval = pthread_mutex_init(&tp->mutex, NULL);
  check_pthread_error(retval, "pthread_mutex_init");

  for (int i = 0; i < numberWorkerThreads; ++i) {
    retval = pthread_create(&tp->workersThreads[i], NULL,
                            threadpool_work_thread, tp);
    check_pthread_error(retval, "threadpool: creating worker thread");
  }

  return 0;
}

int threadpool_submit(threadpool_t *tp, function_t func, void *args)
{
  int retval = pthread_mutex_lock(&tp->mutex);
  check_pthread_error(retval, "threadpool_submit: mutex lock");

  if (tp->shuttingDown) {
    retval = pthread_mutex_unlock(&tp->mutex);
    check_pthread_error(retval, "threadpool_submit: mutex unlock");
    return -1;
  }

  retval = pthread_mutex_unlock(&tp->mutex);
  check_pthread_error(retval, "threadpool_submit: mutex unlock");

  work_item_t *wi = workitem_new(func, args);
  if (wi == END_WORK_ITEM)
    return -1;

  sharedBuffer_Put(&tp->workQueue, wi);

  return 0;
}

void threadpool_destroy(threadpool_t *tp)
{
  int retval = pthread_mutex_lock(&tp->mutex);
  check_pthread_error(retval, "threadpool_destroy: mutex lock");

  tp->shuttingDown = true;

  retval = pthread_mutex_unlock(&tp->mutex);
  check_pthread_error(retval, "threadpool_destroy: mutex unlock");

  for (int i = 0; i < tp->nWorkersThreads; ++i) {
    sharedBuffer_Put(&tp->workQueue, END_WORK_ITEM);
  }

  for (int i = 0; i < tp->nWorkersThreads; ++i) {
    retval = pthread_join(tp->workersThreads[i], NULL);
    check_pthread_error(retval, "pthread_join worker thread");
  }

  free(tp->workersThreads);

  sharedBuffer_destroy(&tp->workQueue);

  retval = pthread_mutex_destroy(&tp->mutex);
  check_pthread_error(retval, "pthread_mutex_destroy");
}
