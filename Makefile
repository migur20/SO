BINS = count_words 

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
	
#ex3: ex3.o
#ex3.o: ex3.c

clean:
	$(RM) $(BINS) *.o
