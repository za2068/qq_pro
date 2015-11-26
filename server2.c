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
	int thread_return;
	char name[NAME_SIZE];
	pthread_t tid;
	struct client_data * next;
} client_data_t;

client_data_t *client_head;
client_data_t *client_last;
int client_sum = 1;
pthread_t join_tid;
pthread_t start_join_tid;
//pthread_t send_message_tid;
pthread_t start_listen_tid;
void *join_return_p;
int join_return;
int server_sd;

void *start_chat(void *p_client_data);
void *start_listen(void *arg);
void *start_join(void *arg);

void *start_listen(void *arg)
{
	int ret;
	ret = pthread_detach(pthread_self());
	if(ret != 0)
	{
		printf("%s:pthread detach failed!\n", __func__);
	}

	while(1)
	{
		int sclient = accept(server_sd, NULL, NULL);
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
}


void *start_join(void *arg)
{
	int ret;

	ret = pthread_detach(pthread_self());
	if(ret != 0)
	{
		printf("%s:pthread detach failed!\n", __func__);
	}

	//zhangao, wrong here
	while(0)
	{
		//will change to create another thread to handle deleting work
		//and it will be detached thread
		pthread_join(join_tid, &join_return_p); 
		memcpy(&join_return, join_return_p, sizeof(int));
		client_data_t *client_front;
		client_data_t *client_p;
		client_front = client_head;
		client_p = client_head->next;

		//find client to delete
		int client_sum_before = client_sum;
		while( client_p->next != NULL)
		{
			if(client_p->tid = join_tid)
			{
				printf("deleting client name = %s, num = %d, tid = %d.\n", 
							client_p->name, client_p->num, (int)client_p->tid);
				client_front->next = client_p->next;
				free(client_p);
				client_sum--;
				break;
			}

			//delete client in the list
			client_front = client_front->next;
			client_p = client_p->next;
		}
		
		if(client_sum == client_sum_before)
		{
			printf("pthread_join:no user to delete,maybe other thread\n");
		}
		else
			printf("delete done\n");
		//need add close sd
	}
}

void *start_chat(void *p_client_data)
{
	int ret;
	int len;
	client_data_t *client_data_p = (client_data_t * ) p_client_data;
	int sclient = client_data_p->sd;

printf("in start_chat \n");

	char buffer[BUF_SIZE];

	ret = pthread_detach(pthread_self());
	if(ret != 0)
	{
		printf("%s:pthread detach failed!\n", __func__);
	}

	/* recv name and send num */
	len = recv(sclient, buffer, BUF_SIZE, 0);
	if (len < 0) {
		printf("recieve name error!\n");
		client_data_p->thread_return = -1;
		return &client_data_p->thread_return;
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
		client_data_p->thread_return = -1;
		return &client_data_p->thread_return;
	}
	len = send(sclient, buffer, sizeof(int), 0);
	if(ret < 0)
	{
		printf("send back error!\n");
		client_data_p->thread_return = -1;
		return &client_data_p->thread_return;
	}

	//printout received message
	while(1) {

		len = recv(sclient, buffer, BUF_SIZE, 0);
printf("get len = %d\n", len);
		if (len < 0) {
			printf("recieve error!\n");
			break;
		}

		buffer[len] = '\0';
		printf("receive[%d]:%s\n", (int)len, buffer);
		if (len > 0) {
			if('q' == buffer[0]) {
				printf("server over!\n");
				break;
			}
				
			char* buffer2 = "I'm a server!";
			len = send(sclient, buffer2, strlen(buffer2), 0);
			if (len < 0)
				printf("send error!\n");
		}
	}
	close(sclient);
	return 0;
}

int main()
{
	char cmd[20];
	client_data_t *client_data_temp;
	client_data_t client_head_data;
	//set server socket address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(MY_PORT);

	//client_head = (client_data_t*)malloc(sizeof(client_data_t));
	
	//*client_head->name = "server";
	client_head = &client_head_data;
	memcpy(client_head->name, "server", sizeof(7));
	client_head->num = 0;
	client_last = client_head;

	if((server_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error!\n");
		exit(0);
	}

	if (bind(server_sd, (struct sockaddr*)&server_addr, 
				sizeof(server_addr)) < 0) {
		printf("bind error!\n");
		exit(0);
	}

	if (listen(server_sd, 128) < 0) {
		printf("listen error!\n");
		exit(0);
	}

	pthread_create(&start_join_tid, NULL, &start_join, NULL);
	//pthread_create(&send_message_tid, NULL, &send_message_interface, NULL);
	pthread_create(&start_listen_tid, NULL, &start_listen, NULL);

	while(1)
	{
		printf("your cmd:");
		scanf("%s", cmd);
		if(strcmp(cmd, "exit") == 0)
		{
			printf("qq_pro is going to be shut down!\n");
			break;
		}
	}

	client_head = client_head->next;
	client_data_temp = client_head;
	while(client_data_temp != NULL)
	{
		printf("free client name=%s,num=%d\n", client_head->name, client_head->num);
		free(client_head);
		client_data_temp = client_data_temp->next;
		client_head = client_data_temp;
	}

	return 0;
}

