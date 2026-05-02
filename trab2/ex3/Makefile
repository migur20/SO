BINS: server client client_stress

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

client_stress: client_stress.o common.o
client_stress.o: client_stress.c

clean:
	$(RM) *.o $(BINS)
