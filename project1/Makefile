CC = g++
C = gcc
CC_FLAGS = -Wall -g -c

server: server.o
	$(CC) server.o -o start_server

client: client.o
	$(CC) client.o -o start_client

server.o: server.cc
	$(CC) $(CC_FLAGS) server.cc -o server.o

client.o: client.cc
	$(CC) $(CC_FLAGS) client.cc -o client.o

clean:
	rm -f *.o *~ start_client start_server connections_log.txt
