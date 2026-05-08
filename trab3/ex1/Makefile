BINS= countdown

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

countdown: countdown.o
countdown.o: countdown.c

clean:
	$(RM) *.o $(BINS)
