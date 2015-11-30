
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define SERVER_PORT 6788
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE	1024
#define NAME_SIZE	20

#define CMD_EXIT "exit"

struct user {
	int num;
	int sd;
	char name[NAME_SIZE];
};

int sd;
pthread_t receive_ptid;

void *start_receive(void *arg);

int main()
{

	struct user my_data;
	ssize_t len;
	int ip;
	inet_pton(AF_INET, SERVER_IP, &ip);

	//set server socket address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ip;
	server_addr.sin_port = htons(SERVER_PORT);

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error!\n");
		exit(0);
	}

	if (connect(sd, (struct sockaddr*)&server_addr,
		sizeof(server_addr)) < 0) {
		printf("connect error!\n");
		exit(0);
	}

	char buffer[BUF_SIZE];

	printf("your name:");
	scanf("%s", my_data.name);

	len = send(sd, my_data.name, strlen(my_data.name), 0);
	if (len < 0) {
		printf("send error!\n");
		exit(0);
	}

	len = recv(sd, &my_data.num, sizeof(int), 0);
	if (len < 0) {
		printf("recieve error!\n");
		exit(0);
	}

	pthread_create(&receive_ptid, NULL, &start_receive, NULL);

	while(1)
	{
		printf("input:");
		scanf("%s", buffer);
		len = send(sd, buffer, strlen(buffer), 0);
		if (len < 0) {
			printf("send error!\n");
			exit(0);
		}
	}

	return 0;
}

void *start_receive(void *arg)
{
	ssize_t len;
	char buffer[BUF_SIZE];
	while(1)
	{
		len = recv(sd, buffer, BUF_SIZE, 0);
		if (len < 0) {
			printf("recieve error!\n");
			exit(0);
		}

		buffer[len] = '\0';
		if(strcmp(buffer, CMD_EXIT) == 0)
		{
			printf("receive exit signal form server\nexiting...\n");
			pthread_exit(NULL);
		}
		printf("receive[%d]:%s\n", (int)len, buffer);
	}
	pthread_exit(NULL);
}

