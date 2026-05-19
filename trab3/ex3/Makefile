BINS= main

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

sharedbuffer_pthread_cond.o: sharedbuffer_pthread_cond.c sharedbuffer_pthread_cond.h

threadpool.o: threadpool.c threadpool.h 

main: main.o threadpool.o sharedbuffer_pthread_cond.o
main.o: main.c

clean:
	$(RM) *.o $(BINS)
