BINS = vector-seq-threads time server_unix client_unix server_inet client_inet

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

vector-seq-threads: vector-seq-threads.o
vector-seq-threads.o: vector-seq-threads.c

server_unix: server_unix.o
server_unix.o: server_unix.c common.h

client_unix: client_unix.o
client_unix.o: client_unix.c common.h

server_inet: server_inet.o
server_inet.o: server_inet.c common.h

client_inet: client_inet.o
client_inet.o: client_inet.c common.h

time: time.o
time.o: time.c

clean:
	$(RM) $(BINS) *.o