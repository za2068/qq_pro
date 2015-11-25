#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>

#define BUF_SIZE	1024
#define MY_PORT 	6788
#define NAME_SIZE	20


typedef struct client_data {
	int num;
	int sd;
	char name[NAME_SIZE];
	pthread_t tid;
	struct client_data * next;
} client_data_t;

client_data_t *client_head;
client_data_t *client_last;
int client_sum = 0;


void *start_chat(void *p_client_data)
{
	int ret;
	int len;
	client_data_t *client_data_p = (client_data_t * ) p_client_data;
	int sclient = client_data_p->sd;

printf("in start_chat \n");

	char buffer[BUF_SIZE];

	len = recv(sclient, buffer, BUF_SIZE, 0);
	if (len < 0) {
		printf("recieve name error!\n");
		exit(0);
	}
	//client_data_p->name = *buffer
	memcpy(client_data_p->name, buffer, NAME_SIZE);
printf("get name = %s\n", client_data_p->name);
	

	//ret = sprintf(buffer, "%d", client_data_p->num);
	//*buffer = (char*)client_data_p->num;
	memcpy(buffer, &client_data_p->num, sizeof(int));
	if(ret < 0)
	{
		printf("sprintf wrong !\n");
		exit(0);
	}
	len = send(sclient, buffer, sizeof(int), 0);
	if(ret < 0)
	{
		printf("send back error!\n");
		exit(0);
	}
	

	while(1) {

		len = recv(sclient, buffer, BUF_SIZE, 0);
printf("get len = %d\n", len);
		if (len < 0) {
			printf("recieve error!\n");
			close(sclient);
			exit(0);
		}

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
	//*client_head->name = "server";
	memcpy(client_head->name, "server", sizeof(7));
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
		printf("new client !! welcome!!\n");

		//init new chat
		client_data_t *p_client_data;
		p_client_data = (client_data_t*)malloc(sizeof(client_data_t));

		//insert new client_data into link
		p_client_data->sd = sclient;
		p_client_data->num = client_sum;
		client_sum++;//change to atomic operation latter
		client_last->next = p_client_data;
		client_last = p_client_data;

		pthread_create(&p_client_data->tid, NULL, &start_chat, p_client_data);

	}

	return 0;
}

