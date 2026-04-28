BINS: server

CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Werror
CFLAGS+=-g

all: $(BINS)

common.o: common.c

server: server.o common.o
server.o: server.c

clean:
	$(RM) *.o $(BINS)
