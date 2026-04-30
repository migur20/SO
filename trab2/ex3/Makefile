BINS: server client

CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Werror
CFLAGS+=-g

all: $(BINS)

common.o: common.c

server: server.o common.o
server.o: server.c

client: client.o common.o
client.o: client.c

clean:
	$(RM) *.o $(BINS)
