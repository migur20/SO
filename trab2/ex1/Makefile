BINS = server_unix client_unix server_inet client_inet server

CC = gcc
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror
CFLAGS += -g

all: $(BINS)

common.o: common.c common.h

server_unix: server_unix.o common.o
server_unix.o: server_unix.c 

client_unix: client_unix.o common.o
client_unix.o: client_unix.c

server_inet: server_inet.o common.o
server_inet.o: server_inet.c

client_inet: client_inet.o common.o
client_inet.o: client_inet.c

server: server.o common.o
server.o: server.c

clean:
	$(RM) $(BINS) *.o
