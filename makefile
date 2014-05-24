CC     = gcc
CFLAGS =
CLIBS  = -lX11 -lrt

all: starwar-server starwar-client

starwar-server: starwar-server.o
	$(CC) $(CFLAGS) -o starwar-server starwar-server.o

starwar-server.o: starwar-server.c
	$(CC) $(CFLAGS) -c -o starwar-server.o starwar-server.c


starwar-client: starwar-client.o
	$(CC) $(CFLAGS) -o starwar-client starwar-client.o $(CLIBS)

starwar-client.o: starwar-client.c
	$(CC) $(CFLAGS) -c -o starwar-client.o starwar-client.c
