BINS = ex1 ex2

CC       = gcc
CFLAGS  += -Wall    # warnings básicos
CFLAGS  += -Wextra  # warnings extra
CFLAGS  += -Werror  # transforma warnings em erros

all: $(BINS)

ex1: ex1.o
ex1.o: ex1.c

ex2: ex2.o
ex2.o: ex2.c

clean:
	$(RM) $(BINS) *.o
