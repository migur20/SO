BINS = ex1 ex3 count_words vector-seq vector-seq-processes time

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g       #debug flags

all: $(BINS)
	
ex1: ex1.o
ex1.o: ex1.c

ex3: ex3.o
ex3.o: ex3.c

count_words: count_words.o
count_words.o: count_words.c

vector-seq: vector-seq.o
vector-seq.o: vector-seq.c

vector-seq-processes: vector-seq-processes.o
vector-seq-processes.o: vector-seq-processes.c

time: time.o
time.o: time

clean:
	$(RM) $(BINS) *.o
