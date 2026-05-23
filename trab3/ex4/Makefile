BINS= server client

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

common.o: common.c common.h

sharedbuffer_pthread_cond.o: sharedbuffer_pthread_cond.c sharedbuffer_pthread_cond.h

threadpool.o: threadpool.c threadpool.h 

server: server.o common.o threadpool.o sharedbuffer_pthread_cond.o
server.o: server.c

client: client.o common.o
client.o: client.c 

clean:
	$(RM) $(BINS) *.o
