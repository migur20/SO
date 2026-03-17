BINS = ex1 ex2 vector-seq ex3

CC       = gcc
CFLAGS  += -Wall    # warnings básicos
CFLAGS  += -Wextra  # warnings extra
CFLAGS  += -Werror  # transforma warnings em erros

all: $(BINS)

ex1: ex1.o
ex1.o: ex1.c
	
ex2: ex2.o
ex2.o: ex2.c
		
vector-seq: vector-seq.o
vector-seq.o: vector-seq.c
	
ex3: ex3.o
ex3.o: ex3.c

clean:
	$(RM) $(BINS) *.o
