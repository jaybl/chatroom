#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX 256
#define PORT 1259

/*Chatroom server designed for the UCI openlab network. Must be kept running to connect from a client.c */

//Client queue. Username and socket stored for each.
typedef struct {
    int cfd;
    char name[32];
    } client;

client clients[100];
static int clnum = 0;


//Writes a message to every client in the queue. Takes in the message to be sent.
void write_to_all(char* message)
{
	for (int i = 0; i < clnum; ++i)
	{
		//printf("%s(%d): %d\n", clients[i].name, strlen(clients[i].name), clients[i].cfd);
		if (strlen(clients[i].name) != 0 && write(clients[i].cfd, message, strlen(message)) == -1)
			perror("write()"); 
	}

}

//Receives messages form a client. Informs everyone else when they leave. Takes in a void * object from the pthread_create() call in main(). Gets converted into a client object in the function. 
void* handle_client(void *arg)
{
	char buffer[MAX] = {0};
	client* chatter = (client *)arg;

	while(recv(chatter->cfd, buffer, sizeof(buffer), 0) > 0)
	{
		char temp[MAX];
		sprintf(temp, "%s: %s\n", chatter->name, buffer);
	//	printf("%s\n", temp);
		write_to_all(temp);
		memset(buffer, 0, sizeof(buffer));
	}
	sprintf(buffer, "%s has left the chatroom.\n", chatter->name);
	//printf("%s has left the chatroom.\n", clients[num].name);
	memset(chatter->name, 0, sizeof(chatter->name));
	write_to_all(buffer);
	close(chatter->cfd);
	pthread_detach(pthread_self());
}


//Changes a client's name. Takes socket and name. 
char* change_username(int sockfd, char* name)
{
	if (recv(sockfd, name, sizeof(name), 0) < 0)
	{
		fprintf(stderr, "receiving username failed.\n");
		exit(1);
	}

}

//Creates the server socket. Will indefinitely for clients to join and send messages. Must be killed manually to end. Port set to 1259. Writes to all when someone joins. 
int main()
{
	strcpy(clients[clnum].name, "SERVER");
	clients[clnum++].cfd = 1;
	pthread_t thr;
	printf("STARTING SERVER.\n");
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Creating Socket failed. Exiting...\n");
		exit(1);
	}
	printf("CREATING SOCKET.\n");
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		fprintf(stderr, "Binding failed. Exiting...\n");
		exit(1);
	}
	printf("BINDING SUCCEEDED.\n");

	listen(sockfd, 3);
	printf("LISTENING.\n");
	
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in client;
	while(1)
	{
		int clientsock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&len);	
		if(clientsock < 0)
		{
			fprintf(stderr, "Accept Failed.\n");
			exit(1);
		}
		change_username(clientsock, clients[clnum].name);
		clients[clnum].cfd = clientsock;
		char temp[MAX];
		sprintf(temp, "%s has joined the chatroom.\n", clients[clnum].name);
		write_to_all(temp);


		pthread_create(&thr, NULL, &handle_client, (void *)&clients[clnum]);
		clnum++;
		
	}
	close(sockfd);
	return 0;

}
