#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 3000
#define MAXBYTE 10240

int initSocketClient(int __domain, int __type, int __protocol, char hostname[]);
void sendFile(int sock_fd, char *file_name);
struct in_addr *convertDomainToIp(char *hostname);

int main(int argc, char *argv[])
{
	int socketId = initSocketClient(AF_INET, SOCK_STREAM, 0, argv[1]);
	sendFile(socketId, argv[2]);
	close(socketId);
	return 0;
}

int initSocketClient(int __domain, int __type, int __protocol, char hostname[])
{
	int socketId;
	struct sockaddr_in server;

	if ((socketId = socket(__domain, __type, __protocol)) < 0)
	{
		perror("Problem in creating the socket");
		exit(2);
	}
	memset(&server, 0, sizeof(server));
	server.sin_family = __domain;
	server.sin_addr.s_addr = convertDomainToIp(hostname)->s_addr;
	server.sin_port = htons(PORT); //convert to big-endian order

	//Connection of the client to the socket
	if (connect(socketId, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Problem in connecting to the server");
		exit(3);
	}
	return socketId;
};

void sendFile(int socketId, char *file_name)
{
	char *buff = (char *)malloc(sizeof(char) * MAXBYTE);
	FILE *pFile;
	size_t sizeBlock;
	if ((pFile = fopen(file_name, "rb")) == NULL)
	{
		perror("fopen error\n");
	}
	else
	{
		send(socketId, file_name, strlen(file_name) + 1, 0);
		while ((sizeBlock = fread(buff, sizeof(char), MAXBYTE, pFile)) > 0)
		{
			send(socketId, buff, sizeBlock, 0);
		}
		free(buff);
		fclose(pFile);
	}
}
struct in_addr *convertDomainToIp(char *hostname)
{
	struct hostent *server;
	server = gethostbyname(hostname);
	if (server == NULL)
	{
		printf("ERROR, no such host %s\n", hostname);
		exit(-1);
	}
	else
	{
		struct in_addr **addr_list;
		addr_list = (struct in_addr **)server->h_addr_list;
		return addr_list[0];
	}
}