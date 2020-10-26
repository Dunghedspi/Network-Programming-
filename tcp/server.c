#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <signal.h>
#include <pthread.h>

#define LISTMAX 8
#define MAXLINE 10240

typedef struct account
{
	char *username;
	char *password;
	int count;
	int socketID;
	struct sockaddr_in client;
	struct account *next;
} account;

account *root = NULL;
int CreateServer(int __domain, int __type, int __protocol, int PORT);
int readFile();
int winterFile();
void newTheard(void *socketID);

account *login(int socketID);
int send_message(char *s, int socketID);

account *findAccount(char *username);
account *addUser(char *buff, account *root);
int recvMess(int socketID, account *acc);

void catch_ctrl_c_and_exit(int sig)
{
	while (root != NULL)
	{
		printf("\nClose socketfd: %d\n", root->socketID);
		close(root->socketID); // close all socket include server_sockfd
		root = root->next;
	}
	exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		perror("lack of input parameters");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c_and_exit);
	printf("%s %d\n", "PID", getpid());

	int socketIdServer;

	// create new socket
	socketIdServer = CreateServer(AF_INET, SOCK_STREAM, 0, atoi(argv[1]));
	// start server
	listen(socketIdServer, LISTMAX);
	printf("%s\n", "Server running...waiting for connections.");
	readFile();

	while (1)
	{
		struct sockaddr_in client;
		socklen_t clilen = sizeof(client);
		//accept a connection
		int socketID = accept(socketIdServer, (struct sockaddr *)&client, &clilen);
		pthread_t id;

		//create new thread
		if (pthread_create(&id, NULL, (void *)newTheard, (void *)&socketID) != 0)
		{
			perror("Create pthread error!\n");
			exit(EXIT_FAILURE);
		}
	}
	close(socketIdServer);
	return 0;
}

void newTheard(void *socketID)
{
	int socketIdClient = *(int *)socketID;
	char *buff = (char *)malloc(sizeof(char) * MAXLINE);
	printf("%s\n", "Child created for dealing with client requests");
	account *acc = login(socketIdClient);

	recvMess(socketIdClient, acc);
	free(buff);
	close(socketIdClient);
}

int CreateServer(int __domain, int __type, int __protocol, int PORT)
{
	// create address server
	struct sockaddr_in server;

	int socketIdServer = socket(__domain, __type, __protocol);
	if (socketIdServer == -1)
	{
		return -1;
	}
	bzero(&server, sizeof(server));
	server.sin_family = __domain;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	bind(socketIdServer, (struct sockaddr *)&server, sizeof(server));

	return socketIdServer;
}

int readFile()
{
	FILE *ptr = fopen("users.txt", "r");
	char *buff = (char *)malloc(100);
	while (!feof(ptr))
	{
		fgets(buff, 100, ptr);
		root = addUser(buff, root);
		memset(buff, 0, 100);
	}
	fclose(ptr);
}

account *addUser(char *buff, account *root)
{
	account *newUser = (account *)malloc(sizeof(account));
	newUser->socketID = 0;
	newUser->username = (char *)malloc(100);
	newUser->password = (char *)malloc(100);
	newUser->next = NULL;
	char *token = strtok(buff, " ");
	strcpy(newUser->username, token);
	strcpy(newUser->password, strtok(NULL, " "));
	newUser->count = atoi(strtok(NULL, " "));
	if (root == NULL)
	{
		root = newUser;
	}
	else
	{
		account *cur = root;
		while (cur->next != NULL)
		{
			cur = cur->next;
		}
		cur->next = newUser;
	}
	return root;
}

account *findAccount(char *username)
{
	account *cur = root;
	account *result = NULL;
	while (cur != NULL)
	{
		if (strcmp(cur->username, username) == 0)
		{
			result = cur;
		}
		cur = cur->next;
	}
	return result;
}

int winterFile()
{
	FILE *ptr = fopen("users.txt", "w+");
	if (root != NULL)
	{
		account *cur = root;
		while (cur != NULL)
		{
			fprintf(ptr, "%s %s %d", cur->username, cur->password, cur->count);
			if (cur->next != NULL)
				fprintf(ptr, "%s", "\n");
			cur = cur->next;
		}
	}
	fclose(ptr);
}

account *login(int socketID)
{
	account *user = NULL;
	char *buff = (char *)malloc(MAXLINE);
	int isLogin = 0;
	while (isLogin == 0)
	{
		recv(socketID, buff, MAXLINE, 0);
		puts(buff);
		if (strncmp(buff, "username", 8) == 0)
		{
			account *acc = findAccount(buff + 9);
			if (acc != NULL)
			{
				send(socketID, "Insert Password", 17, 0);
				memset(buff, 0, strlen(buff));
				while (isLogin == 0)
				{
					recv(socketID, buff, MAXLINE, 0);
					if (strcmp(buff + 9, acc->password) == 0)
					{
						if (acc->count < 3)
						{
							isLogin = 1;
							user = acc;
							acc->socketID = socketID;
							send(socketID, "OK", 17, 0);
							acc->count = 0;
						}
						else
						{
							send(socketID, "Account not ready", 20, 0);
							memset(buff, 0, strlen(buff));
							break;
						}
					}
					else
					{
						acc->count++;
						send(socketID, "Not OK", 17, 0);
						memset(buff, 0, strlen(buff));
						continue;
					}
				}
			}
			else
			{
				send(socketID, "Wrong account", 16, 0);
			}
			memset(buff, 0, strlen(buff));
		}
	}
	free(buff);
	return user;
}

int recvMess(int socketID, account *acc)
{
	char *buff = (char *)malloc(sizeof(char) * MAXLINE);
	memset(buff, 0, strlen(buff));
	int leave = 0;
	while (leave == 0)
	{
		recv(socketID, buff, MAXLINE, 0);
		if (strncmp(buff, "changepass", 10) == 0)
		{
			memset(acc->password, 0, strlen(acc->password));
			strcpy(acc->password, buff + 11);
			winterFile();
			char *numberlist = (char *)malloc(100);
			char *charlist = (char *)malloc(100);
			int k = 0;
			int l = 0;
			for (int i = 0; i < strlen(acc->password); i++)
			{
				if (acc->password[i] <= '9' && acc->password[i] >= '0')
				{
					numberlist[k++] = acc->password[i];
				}
				else
				{
					charlist[l++] = acc->password[i];
				}
			}
			memset(buff, 0, strlen(buff));
			numberlist[k] = '\0';
			charlist[l] = '\0';
			strcat(buff, strcat(numberlist, charlist));
			free(numberlist);
			free(charlist);
			send(socketID, buff, strlen(buff), 0);
		}
		else
		{
			if (strncmp(buff, "bye", 3) == 0)
			{
				memset(buff, 0, strlen(buff));
				leave = 1;
				break;
			}
			else
			{
				send_message(buff, socketID);
			}
		}
		memset(buff, 0, strlen(buff));
	}
	close(socketID);
	free(buff);
}
int send_message(char *s, int socketID)
{
	if (root != NULL)
	{
		account *cur = root;
		while (cur != NULL)
		{
			if (cur->socketID != socketID)
			{
				if (write(cur->socketID, s, strlen(s)) < 0)
				{
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
			cur = cur->next;
		}
	}
}