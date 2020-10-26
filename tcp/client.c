#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 10240
int client_sock_fd;
int login();
int changePass(int socketID);
int sendText(int socketID);
int sendMess(int socketID);
char userName[100];

char buff[MAX_LINE];
void signio_handler(int signo)
{
	memset(buff, 0, MAX_LINE);
	size_t n = recv(client_sock_fd, buff, sizeof buff, 0);
	puts(buff);
}

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		perror("lack of input parameters");
		exit(-1);
	}
	struct sockaddr_in client_socket;
	struct sockaddr_in server_socket;
	client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	server_socket.sin_family = AF_INET;
	server_socket.sin_port = htons(atoi(argv[2]));
	server_socket.sin_addr.s_addr = inet_addr(argv[1]);
	printf("server IP = %s ", inet_ntoa(server_socket.sin_addr));

	if (connect(client_sock_fd, (struct sockaddr *)&server_socket, sizeof(server_socket)) < 0)
	{
		printf("Error in connecting to server\n");
		exit(0);
	}
	else
		printf("connected to server\n");

	// Signal driven I/O mode and NONBlOCK mode so that recv will not block
	if (fcntl(client_sock_fd, F_SETFL, O_NONBLOCK | O_ASYNC))
		printf("Error in setting socket to async, nonblock mode");

	signal(SIGIO, signio_handler); // assign SIGIO to the handler

	//set this process to be the process owner for SIGIO signal
	if (fcntl(client_sock_fd, F_SETOWN, getpid()) < 0)
		printf("Error in setting own to socket");

	login();
	sendMess(client_sock_fd);
	close(client_sock_fd);
}

int sendMess(int socketID)
{
	int c;
	do
	{
		printf("1.Change Password\n2.Send Message\nEnter:");
		scanf("%d", &c);
		while (getchar() != '\n')
			;
		switch (c)
		{
		case 1:
			changePass(socketID);
			break;

		case 2:
			sendText(socketID);
			break;
		}
	} while (c == 1);
}

int changePass(int socketID)
{
	char *newPass = (char *)malloc(100);
	int c = 1;
	do
	{
		printf("Enter new Password:");
		fgets(newPass, 100, stdin);
		newPass[strlen(newPass) - 1] = '\0';
		for (int i = 0; i < strlen(newPass); i++)
		{
			if (!((newPass[i] <= 'Z' && newPass[i] >= 'A') || (newPass[i] <= 'z' && newPass[i] >= 'a') || (newPass[i] <= '9' && newPass[i] >= '0')))
			{
				c = 0;
				break;
			}
		}
		if (c == 0)
			printf("Invalid password\n");
	} while (c != 1);
	char *buff1 = (char *)malloc(MAX_LINE);
	strcat(buff1, "changepass:");
	strcat(buff1, newPass);
	send(socketID, buff1, strlen(buff1), 0);
	memset(buff1, 0, strlen(buff));
	ssize_t n = recv(socketID, buff1, strlen(buff1), 0);
	for (int i = 0; i < strlen(newPass); i++)
	{
		if (!((newPass[i] <= 'Z' && newPass[i] >= 'A') || (newPass[i] <= 'z' && newPass[i] >= 'a') || (newPass[i] <= '9' && newPass[i] >= '0')))
		{
			memset(buff1, 0, strlen(buff1));
			strcpy(buff1, "Error");
			send(socketID, buff1, strlen(buff1), 0);
		}
	}
	printf("%s\n", buff1);
	free(newPass);
	free(buff1);
}

int sendText(int socketID)
{
	char *text = (char *)malloc(MAX_LINE);
	while (1)
	{

		fgets(text, MAX_LINE, stdin);
		text[strlen(text) - 1] = '\0';
		strcat(text, "(from: ");
		strcat(text, userName);
		strcat(text, ") ");
		if (text != NULL)
		{
			send(socketID, text, strlen(text), 0);
			if (strncmp(text, "bye", 3) == 0)
			{
				printf("Goodbye %s\n", userName);
				free(text);
				return 1;
			}
			memset(text, 0, strlen(text));
		}
		else
		{
			break;
		}
	}
	free(text);
}

int login()
{
	int isLogin = 0;
	char *username = (char *)malloc(100);
	char *password = (char *)malloc(100);
	while (isLogin == 0)
	{

		printf("username:");
		scanf("%s", username);
		char *sendText = (char *)malloc(MAX_LINE);
		strcat(sendText, "username:");
		strcat(sendText, username);
		send(client_sock_fd, sendText, strlen(sendText), 0);
		sleep(1);
		if (strlen(buff) > 0)
		{
			if (strncmp(buff, "Insert Password", 15) == 0)
			{
				memset(buff, 0, strlen(buff));
				while (isLogin == 0)
				{
					printf("Password:");
					scanf("%s", password);
					memset(sendText, 0, strlen(sendText));
					strcat(sendText, "password:");
					strcat(sendText, password);
					send(client_sock_fd, sendText, strlen(sendText), 0);
					sleep(1);
					if (strlen(buff) > 0)
					{
						if (strncmp(buff, "OK", 2) == 0)
						{
							isLogin = 1;
							strcpy(userName, username);
						}
						else
						{
							if (strncmp(buff, "Not OK", 6) == 0)
							{
								printf("Insert Password\n");
								memset(buff, 0, strlen(buff));
								continue;
							}
						}
					}
					break;
				}
			}
		}
	}
	memset(buff, 0, strlen(buff));
	free(username);
	free(password);
}