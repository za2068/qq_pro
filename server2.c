#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE	1024
#define MY_PORT 	6788

client_data_t *client_head;
client_data_t *client_last;
int client_sum = 0;

typedef struct client_data {
	int num;
	int sd;
	char name[20];
	pthread_t tid;
	struct client_data * next;
} client_data_t;


void *start_chat(void *p_client_data)
{
	client_data_t *client_data_p = (client_data_t * ) p_client_data;
	int sclient = client_data_p->sd;
	while(1) {

		char buffer[BUF_SIZE];
		ssize_t len = recv(sclient, buffer, BUF_SIZE, 0);
		if (len < 0) {
			printf("recieve error!\n");
			close(sclient);
			continue;
				buffer[len] = '\0';
		printf("receive[%d]:%s\n", (int)len, buffer);
		if (len > 0) {
			if('q' == buffer[0]) {
				printf("server over!\n");
				exit(0);
			}
				
			char* buffer2 = "I'm a server!";
			len = send(sclient, buffer2, strlen(buffer2), 0);
			if (len < 0)
				printf("send error!\n");
		}
	}
	close(sclient);
}

int main() {

	//set server socket address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(MY_PORT);

	client_head = (client_data_t*)malloc(sizeof(client_data_t));
	client_head->name = "server";
	client_head->num = 0;
	client_last = client_head;

	int sd;
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error!\n");
		exit(0);
	}

	if (bind(sd, (struct sockaddr*)&server_addr, 
				sizeof(server_addr)) < 0) {
		printf("bind error!\n");
		exit(0);
	}

	if (listen(sd, 128) < 0) {
		printf("listen error!\n");
		exit(0);
	}

	while(1)
	{
		int sclient = accept(sd, NULL, NULL);
		if (sclient < 0) {
			printf("accept error!\n");
			exit(0);
		}

		//init new chat
		client_data_t *p_client_data;
		p_client_data = (client_data_t*)malloc(sizeof(client_data_t));

		//insert new client_data into link
		p_client_data->sd = sclient;
		p_client_data = client_last->next;
		client_last = p_client_data;

		pthread_create(&p_client_data->tid, NULL, &start_chat, p_client_data);
	}

	return 0;
}

