CC = g++ -pthread
C = gcc
CFLAGS = -Wall -g -c 
DEPS = socket.h topic_node.h

%.o: %.cc $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: server.o socket.o tnode.o
	$(CC) server.o socket.o tnode.o -o start_server

client: client.o socket.o
	$(CC) client.o socket.o -o start_client

clean:
	rm -f *.o *~ start_client start_server connections_log.txt client_connections_log*
