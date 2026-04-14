BINS = vector-seq-threads server_unix client_unix

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

vector-seq-threads: vector-seq-threads.o
vector-seq-threads.o: vector-seq-threads.c

server_unix: server_unix.o
server_unix.o: server_unix.c

client_unix: client_unix.o
client_unix.o: client_unix.c

clean:
	$(RM) $(BINS) *.o