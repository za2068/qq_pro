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

#define CMD_EXIT "exit"
#define CMD_PRINT_LIST "print_list"
//#define CMD_NUM_EXIT 0


typedef struct client_data {
	pthread_t tid;
	int num;
	int sd;
	char name[NAME_SIZE];
	struct client_data * next;
	//int thread_return;
} client_data_t;

client_data_t *client_head;
client_data_t *client_last;
int client_sum = 0; 
int client_present_num = 0; //it will be the client num
//pthread_t join_tid;
//pthread_t start_join_tid;
//pthread_t send_message_tid;
//void *join_return_p;
//int join_return;
//int listen_return;
pthread_t listen_tid;
pthread_t server_surface_tid;
int server_sd;
pthread_mutex_t client_mutex; 
pthread_mutex_t contrl_mutex; 
pthread_cond_t contrl_cond;
int is_exit_status = 0;

void *start_chat(void *p_client_data);
void *start_listen(void *arg);
//void *start_join(void *arg);
void *start_server_surface(void *arg);
void shutdown_service();
void print_list();

void close_all_pthread();
void free_all_client();
void free_all_mutex();
void free_all_cond();

void *start_listen(void *arg)
{
	int ret;
	ret = pthread_detach(pthread_self());
	if(ret != 0)
	{
		printf("%s:pthread detach failed!\n", __func__);
	}
	ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	
	if(ret != 0)
	{
		printf("%s:pthread set asynchronous failed!\n", __func__);
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

		ret = pthread_mutex_lock(&client_mutex);
		if(ret)
		{
			printf("%s:pthread client_mutex lock failed\n", __func__);
			pthread_exit(NULL);
		}
		client_sum++;
		client_present_num++;
		ret = pthread_mutex_unlock(&client_mutex);
		if(ret)
		{
			printf("%s:pthread client_mutex unlock failed\n", __func__);
			pthread_exit(NULL);
		}

		client_last->next = p_client_data;
		//insert new client_data into link
		p_client_data->sd = sclient;
		p_client_data->num = client_present_num;
		//// need thread synchronization
		
		client_last = p_client_data;

		pthread_create(&p_client_data->tid, NULL, &start_chat, p_client_data);
		//print_list();

		//start pthread_join if not started
	}
}

void delete_client(pthread_t tid)
{
	int ret;

//	ret = pthread_detach(pthread_self());
//	if(ret != 0)
//	{
//		printf("%s:pthread detach failed!\n", __func__);
//	}
//printf("in start_join\n");
//
//	////zhangao, wrong here
//	while(1)
//	{
//		////will change to create another thread to handle deleting work
//		//and it will be detached thread
//		pthread_join(join_tid, NULL); 
//		//memcpy(&join_return, join_return_p, sizeof(int));

	client_data_t *client_front;
	client_data_t *client_p;
	client_front = client_head;
	client_p = client_head->next;

printf("deleting tid = %d\n", (int)tid);
	// lock for client_sum
	ret = pthread_mutex_lock(&client_mutex);
	if(ret)
	{
		printf("%s:pthread client_mutex lock failed\n", __func__);
		pthread_exit(NULL);
	}
	/* find client to delete */
	int client_sum_before = client_sum;
	while( client_p != NULL)
	{
		//delete client in the list,and close socket
		if(client_p->tid == tid)
		{
			printf("deleting client name = %s, num = %d, tid = %d.\n", 
					client_p->name, client_p->num, (int)client_p->tid);
			if(client_p->next == NULL)
			{
				client_last = client_front;
				client_last->next = NULL;
			}
			else
				client_front->next = client_p->next;
			close(client_p->sd);
			free(client_p);
			client_sum--;
			break;
		}

		client_front = client_front->next;
		client_p = client_p->next;
	}
	
	if(client_sum == client_sum_before)
	{
		printf("pthread_join:no user to delete,wrong or other thread end\n");
	}
	else
		printf("delete done\n");
	
	ret = pthread_mutex_unlock(&client_mutex);
	if(ret)
	{
		printf("%s:pthread client_mutex unlock failed\n", __func__);
		pthread_exit(NULL);
	}
	print_list();
}

void print_list()
{
	client_data_t *client_temp;
	client_temp = client_head;

	printf("client_sum = %d\n", client_sum);
	while(client_temp != NULL)
	{
		printf("name: %s, num: %d, tid = %d \n", 
			client_temp->name, client_temp->num, (int)client_temp->tid);
		client_temp = client_temp->next;
	}
	return ;
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
	ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	
	if(ret != 0)
	{
		printf("%s:pthread set asynchronous failed!\n", __func__);
	}

	/* recv name and send num */
	len = recv(sclient, buffer, BUF_SIZE, 0);
	if (len < 0) {
		printf("recieve name error!\n");
		delete_client(client_data_p->tid);
		close(sclient);
		pthread_exit(NULL);
	}
	//client_data_p->name = *buffer
	memcpy(client_data_p->name, buffer, NAME_SIZE);
printf("get name = %s\n", client_data_p->name);


	//ret = sprintf(buffer, "%d", client_data_p->num);
	//*buffer = (char*)client_data_p->num;
	memcpy(buffer, &client_data_p->num, sizeof(int));

	len = send(sclient, buffer, sizeof(int), 0);
	if(ret < 0)
	{
		printf("send back error!\n");
		delete_client(client_data_p->tid);
		close(sclient);
		pthread_exit(NULL);
	}

	/* printout received message */
	while(1) {
printf("in start_chat_thread \n");
		len = recv(sclient, buffer, BUF_SIZE, 0);
printf("get len = %d\n", len);
		if (len < 0) {
			printf("recieve error!\n");
			break;
		}
		else if (len == 0)
		{
			printf("client disconnected!\nexiting!\n");
			delete_client(client_data_p->tid);
			close(sclient);
			pthread_exit(NULL);
		}

		buffer[len] = '\0';
		printf("receive[%d]:%s\n", (int)len, buffer);
		if (len > 0) {
			if(strcmp(buffer, CMD_EXIT) == 0) {
				printf("server over!\n");
				delete_client(client_data_p->tid);
				close(sclient);
				pthread_exit(NULL);
			}
				
			char* buffer2 = "I'm a server!";
			len = send(sclient, buffer2, strlen(buffer2), 0);
			if (len < 0)
			{
				printf("send error!\n");
				break;
			}
		}
	}
	delete_client(client_data_p->tid);
	close(sclient);
	return 0;
}

int main()
{
	int ret,cmd_number;

	/* set server socket address */
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(MY_PORT);

	ret = pthread_mutex_init(&client_mutex, NULL);  
	if(ret)
	{
		printf("init client_mutex failed\n");
		return -1;
	}
	ret = pthread_mutex_init(&contrl_mutex, NULL);  
	if(ret)
	{
		printf("init contrl_mutex failed\n");
		return -1;
	}
	ret = pthread_cond_init(&contrl_cond, NULL);  
	if(ret)
	{
		printf("init contrl_cond failed\n");
		return -1;
	}

	client_head = (client_data_t*)malloc(sizeof(client_data_t));
	
	memcpy(client_head->name, "server", sizeof(7));
	client_head->num = client_present_num;
	client_last = client_head;

	/* server_sd will be closed at function free_all_client */
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

	//pthread_create(&send_message_tid, NULL, &send_message_interface, NULL);
	//pthread_create(&start_join_tid, NULL, &start_join, NULL);
	pthread_create(&listen_tid, NULL, &start_listen, NULL);
	pthread_create(&server_surface_tid, NULL, &start_server_surface, NULL);

	while(1)
	{
		pthread_mutex_lock(&contrl_mutex);
		pthread_cond_wait(&contrl_cond, &contrl_mutex);
		if(is_exit_status)
		{
			//shutdown_service();
			
			/* free client and the thread at the same time */
			close_all_pthread();
			free_all_client();

		}
		pthread_mutex_unlock(&contrl_mutex);
		// do some detail works later
		if(is_exit_status)
		{
			break;
		}
	}

	free_all_mutex();
	free_all_cond();

	return 0;
}

void close_all_pthread()
{
printf("free pthreads.. \n");
	if(pthread_cancel(listen_tid))
	{
		printf("pthread_cancel failed, tid = %d \n", (int)listen_tid);
	}
	if(pthread_cancel(server_surface_tid))
	{
		printf("pthread_cancel failed, tid = %d \n", (int)server_surface_tid);
	}

	return;
}
void free_all_client()
{
	ssize_t len;
	int ret;

	client_data_t *client_data_temp;
printf("free clients...\n");

	client_data_temp = client_head;
	client_data_temp = client_data_temp->next;

	while(client_data_temp != NULL)
	{
		/* 1. send shutdown signal to client */
		printf("send shutdown signal to %s, num = %d\n", 
					client_data_temp->name, client_data_temp->num);
		len = send(client_data_temp->sd, CMD_EXIT, strlen(CMD_EXIT), 0);
		if(len < 0)
		{
			printf("send shutdown signal to %s failed\n",client_data_temp->name);
		}

		/* 2. close thread */
		if(pthread_cancel(client_data_temp->tid))
		{
			printf("pthread_cancel failed, tid = %d \n", (int)client_data_temp->tid);
		}

		/* 3. close socket connected with client */
		close(client_data_temp->sd);

		/* 4. free client mem */
		printf("free client name=%s,num=%d\n", client_data_temp->name, client_data_temp->num);
		free(client_data_temp);
		
		client_data_temp = client_data_temp->next;
	}

	printf("free client head(server)\n");
	close(server_sd);
	free(client_head);

	return;
}
void free_all_mutex()
{
printf("free mutexs...\n");
	if(pthread_mutex_destroy(&client_mutex))
	{
		printf("client_mutex destroy failed\n");
		return;
	}
	if(pthread_mutex_destroy(&contrl_mutex))
	{
		printf("contrl_cond destroy failed\n");
		return;
	}
	return;
}
void free_all_cond()
{
printf("free conds... \n");
	if(pthread_cond_destroy(&contrl_cond))
	{
		printf("contrl_cond destroy failed\n");
		return;
	}
	return;
}

void *start_server_surface(void *arg)
{
	char cmd[20];
	if(pthread_detach(pthread_self()))
	{
		printf("%s:pthread detach failed!\n", __func__);
	}
	if(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL))
	{
		printf("set pthread cancel type failed\n");
		return;
	}
	while(1)
	{
		printf("your cmd:");
		scanf("%s", cmd);
		fflush(stdin);

		if(strcmp(cmd, CMD_EXIT) == 0)
		{
			printf("qq_pro is going to be shut down!\n");
			pthread_mutex_lock(&contrl_mutex);
			/* set exit status */
			is_exit_status = 1;
			pthread_cond_signal(&contrl_cond);
			//pthread_cond_boardcast(&contrl_cond);
			//shutdown_service();
			pthread_mutex_unlock(&contrl_mutex);

			break;
		}
		if(strcmp(cmd, CMD_PRINT_LIST) == 0)
		{
			print_list();
		}
		////need end other thing like end pthread
		//
	}
}

