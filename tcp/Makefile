CC=gcc
CFLAGS=-O3 -Wall
LIBS=-pthread

all: server client
server: server.o
	$(CC) $(CFLAGS) $(LIBS) -o server server.o
client: client.o
	$(CC) $(CFLAGS) -o client client.o
server.o: server.c
	$(CC) $(CFLAGS) -c server.c
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

.PHONY: clean
clean:
	rm -f *.o *.out