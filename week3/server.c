#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include "utils.h"

#define PORT 3000
#define LISTMAX 8
#define MAXLINE 10240

typedef struct fileImage
{
	char fileName[100];
	char *data;
} fileImage;

int CreateServer(int __domain, int __type, int __protocol);

int recvFile(int socketIdConnect)
{
	char *type = (char *)malloc(sizeof(char) * 10);
	char *buf = (char *)malloc(MAXLINE);
	FILE *ptr;
	if (recv(socketIdConnect, buf, MAXLINE, 0) > 0)
	{
		int i;
		for (i = strlen(buf) - 1; i >= 0; i--)
		{
			if (buf[i] == '.')
			{
				strcpy(type, buf + i);
				buf[i] = '\0';
				break;
			}
		}
		strcat(buf, getLocalTime());
		strcat(buf, type);
		ptr = fopen(buf, "wb+");
		if (ptr == NULL)
		{
			perror("Can not create file");
			exit(0);
		}
		memset(buf, '\0', MAXLINE);
		size_t n;
		while ((n = recv(socketIdConnect, buf, MAXLINE, 0)) > 0)
		{
			fwrite(buf, sizeof(char), n, ptr);
			memset(buf, '\0', n);
		}

		fclose(ptr);
	}
	fwrite(buf, sizeof(char), MAXLINE, ptr);
	free(buf);
	return 1;
}

int main(int argc, char const *argv[])
{

	int socketIdServer, socketIdClient, n;

	// create new socket
	socketIdServer = CreateServer(AF_INET, SOCK_STREAM, 0);
	// start server
	listen(socketIdServer, LISTMAX);
	printf("%s\n", "Server running...waiting for connections.");

	struct sockaddr_in client;
	socklen_t clilen;

	for (;;)
	{
		clilen = sizeof(client);
		socketIdClient = accept(socketIdServer, (struct sockaddr *)&client, &clilen);
		printf("%s\n", "Received request...");
		recvFile(socketIdClient);
		close(socketIdClient);
	}
	return 0;
}

int CreateServer(int __domain, int __type, int __protocol)
{
	// create address server
	struct sockaddr_in server;

	int socketIdServer = socket(__domain, __type, __protocol);
	if (socketIdServer == -1)
	{
		return -1;
	}

	server.sin_family = __domain;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	bind(socketIdServer, (struct sockaddr *)&server, sizeof(server));

	return socketIdServer;
}
