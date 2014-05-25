CC     = gcc
CFLAGS = -O2
CLIBS1 = -lX11 -lrt
CLIBS2 = -lrt

all: starwar-server starwar-client

starwar-server: starwar-server.o
	$(CC) $(CFLAGS) -o starwar-server starwar-server.o $(CLIBS2)

starwar-server.o: starwar-server.c
	$(CC) $(CFLAGS) -c -o starwar-server.o starwar-server.c


starwar-client: starwar-client.o
	$(CC) $(CFLAGS) -o starwar-client starwar-client.o $(CLIBS1)

starwar-client.o: starwar-client.c
	$(CC) $(CFLAGS) -c -o starwar-client.o starwar-client.c

clean:
	rm *.o
	rm starwar-server
	rm starwar-client
