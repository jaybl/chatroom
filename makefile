
server: server.o
	gcc -ggdb -std=c99 -pthread server.o -o server

server.o: server.c
	gcc -ggdb -std=c99 -c -pthread server.c
client: client.c
	gcc -ggdb client.c -std=c99 -o client

