#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>

void check_pthread_error(int retval, const char *msg)
{
  if (retval < 0) {
		perror(msg);
		exit(EXIT_FAILURE);
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
    if (wi == NULL)
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
int threadpool_init(threadpool_t *tp, int workQueueDim, int numberWorkerThreads)
{
  if (sharedBuffer_init(&tp->workQueue, workQueueDim) < 0)
    return -1;
  tp->workersThreads = malloc(numberWorkerThreads * sizeof(pthread_t));
  if (tp->workersThreads == NULL) {
    sharedBuffer_destroy(&tp->workQueue);
    return -1;
  }
  tp->nWorkersThreads = numberWorkerThreads;
  for (int i = 0; i < numberWorkerThreads; ++i) {
    int retval = pthread_create(&tp->workersThreads[i], NULL,
                                threadpool_work_thread, tp);
    check_pthread_error(retval, "threadpool: creating worker thread");
  }
  return 0;
}

int threadpool_submit(threadpool_t *tp, function_t func, void *args)
{
  work_item_t *wi = workitem_new(func, args);
  if (wi == NULL) {
    printf("workitem create failled");
    return -1;
  }
  sharedBuffer_Put(&tp->workQueue, wi);
  return 0;
}

void threadpool_destroy(threadpool_t *tp)
{
  // to be implemented
}
