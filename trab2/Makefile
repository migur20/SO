BINS = vector-seq-threads time

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g       #debug flags

all: $(BINS)

vector-seq-threads: vector-seq-threads.o
vector-seq-threads.o: vector-seq-threads.c

time: time.o
time.o: time.c

clean:
	$(RM) $(BINS) *.o
