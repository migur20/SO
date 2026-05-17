BINS= countdown

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

countdown: main.o countdown.o
main.o: main.c countdown.h
countdown.o: countdown.c countdown.h

clean:
	$(RM) *.o $(BINS)
