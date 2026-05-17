BINS= pool

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g
CFLAGS += -pthread

all: $(BINS)

pool: pool.o threadpool.o sharedbuffer_pthread_cond.o
	$(CC) $(CFLAGS) -o pool pool.o threadpool.o sharedbuffer_pthread_cond.o

pool.o: pool.c threadpool.h sharedbuffer_pthread_cond.h
threadpool.o: threadpool.c threadpool.h sharedbuffer_pthread_cond.h
sharedbuffer_pthread_cond.o: sharedbuffer_pthread_cond.c sharedbuffer_pthread_cond.h

clean:
	$(RM) *.o $(BINS)
