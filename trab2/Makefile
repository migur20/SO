<<<<<<< HEAD
BINS = vector-seq-threads server_unix client_unix
=======
BINS = vector-seq-threads time
>>>>>>> 1675d229a7636f97769ddd3e2f2fa36903c4797a

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

vector-seq-threads: vector-seq-threads.o
vector-seq-threads.o: vector-seq-threads.c

<<<<<<< HEAD
server_unix: server_unix.o
server_unix.o: server_unix.c

client_unix: client_unix.o
client_unix.o: client_unix.c
=======
time: time.o
time.o: time.c
>>>>>>> 1675d229a7636f97769ddd3e2f2fa36903c4797a

clean:
	$(RM) $(BINS) *.o