<<<<<<< HEAD
BINS = count_words time

CC       = gcc
CFLAGS  += -Wall    # warnings básicos
CFLAGS  += -Wextra  # warnings extra
CFLAGS  += -Werror  # transforma warnings em erros

all: $(BINS)

#ex1: ex1.o
#ex1.o: ex1.c
	
ex2: ex2.o
ex2.o: ex2.c

count_words: count_words.o
	$(CC) $(CFLAGS) -o count_words count_words.o
		
#vector-seq: vector-seq.o
#vector-seq.o: vector-seq.c

time: time.o time.c
	$(CC) $(CFLAGS) -o time time.o
	
#ex3: ex3.o
#ex3.o: ex3.c

clean:
	$(RM) $(BINS) *.o
=======
BINS = ex1 ex3 count_words vector-seq vector-seq-processes

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

clean:
	$(RM) $(BINS) *.o
>>>>>>> 8a5a0e7de6bde3c3d3dfbd112e1e1e069339a1e6
