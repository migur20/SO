BINS= pool

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

pool: pool.o
pool.o: pool.c

clean:
	$(RM) *.o $(BINS)
