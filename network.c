#include "network.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include <netinet/in.h>
#include<string.h>

int socket_desc;
struct sockaddr_in server;

int init_network(char* IP, int port)
{
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
		return -1;
	}
	
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );
	
	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return -1;
	}
	
	return 0;
}

int send_net_message(char* message)
{
	if( send(socket_desc , message , strlen(message) , 0) < 0)
	{
		puts("Send failed");
		return -1;
	}
}