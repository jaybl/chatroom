#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h> 
#include <sys/time.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

#define MAX 256

/* Program that connects to a server hosted by chatroom.c. Must enter username, hostname, and port. Designed to work for the openlab system at UCI only. 
EXAMPLE INPUT: client Tim rip-riley.ics.uci.edu 1259*/

//checks if hostname given was valid. Takes in hostname and hostent struct. 
void get_host(char* host, struct hostent* he)
{
	if ((he = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unable to connect to host. \n");
		exit(1);
	}
}

//creates the socket and returns it. 
int get_socket()
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Unable to create socket. \n");
		exit(1);
	}
	return sockfd;
}

//recieves and sends messages to the chatroom server until "quit" is entered by the client. Takes socket and returns 1. 
int chat_room(int sockfd)
{
	fd_set fds;
	int maxfd = (sockfd > 0) ? sockfd : 0;
	while(1)
	{
		char buffer[MAX] = {0};
		char receive[MAX] = {0};
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		FD_SET(0, &fds);
		select(maxfd + 1, &fds, NULL, NULL, NULL);
		if (FD_ISSET(0, &fds))
		{
			if(fgets(buffer, sizeof(buffer), stdin) == NULL || !strcmp(buffer, "quit\n"))
				break;
			buffer[strcspn(buffer, "\n")] = 0;
			if (send(sockfd, buffer, strlen(buffer), 0) < 0)
			{
				fprintf(stderr, "Send failed. \n"); 
				break;
			}
		}
		if (FD_ISSET(sockfd, &fds))
		{
			if (recv(sockfd, receive, sizeof(receive), 0) < 0)
			{
				fprintf(stderr, "Receive failed. \n"); 
				break;
			}
			printf("%s", receive);
		}
	}
	close(sockfd);
	return 1;

}

//Sends username to the server. Takes the username and server socket.
int send_username(char* username, int sockfd)
{
	if (send(sockfd, username, strlen(username), 0) < 0)
	{
		fprintf(stderr, "Send failed. \n"); 
		close(sockfd);
		return 0;
	}
}

//Takes in username, host, and port. Prints them out and starts chatroom().
int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Not enough arguments. Exiting now...\n");
		exit(-1);
	}
	char* username = argv[1];
	char* host = argv[2];
	char* port = argv[3];

	struct hostent *he; 
	he = gethostbyname(argv[2]);
	int sockfd = get_socket();
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(port));
	server_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
	
	if(connect(sockfd, (struct sockaddr *)& server_addr, sizeof(server_addr))  < 0)
	{
		fprintf(stderr, "Unable to connect to server. \n");
		exit(1);
	}
	
	printf("username: %s\n", username);
	printf("host    : %s\n", host);
	printf("port    : %s\n\n", port);
	
	send_username(argv[1], sockfd);
	chat_room(sockfd);
	
	return 0;
}
