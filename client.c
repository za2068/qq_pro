
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
pthread_t contrl_ptid;
pthread_mutex_t contrl_mutex;
pthread_cond_t contrl_cond;
int is_exit_status = 0;

char buffer[BUF_SIZE];

void *start_receive(void *arg);
void *start_contrl(void *arg);

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
	pthread_create(&contrl_ptid, NULL, &start_contrl, NULL);

	while(1)
	{
		pthread_mutex_lock(&contrl_mutex);
		pthread_cond_wait(&contrl_cond, &contrl_mutex);
		if(is_exit_status)
		{
			if(pthread_cancel(receive_ptid))
			{
				printf("pthread_receive 1cancel failed, tid = %d \n", (int)receive_ptid);
			}
			if(pthread_cancel(contrl_ptid))
			{
				printf("pthread_receive 1cancel failed, tid = %d \n", (int)contrl_ptid);
			}
			break;
		}
		pthread_mutex_unlock(&contrl_mutex);
	}

	if(pthread_mutex_destroy(&contrl_mutex))
	{
		printf("destory contrl_mutex failed\n");
	}
	if(pthread_cond_destroy(&contrl_cond))
	{
		printf("destory contrl_cond failed\n");
	}
	/* close socket */
	close(sd);

	return 0;
}

void *start_contrl(void *arg)
{
	ssize_t len;

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
		printf("input:");
		scanf("%s", buffer);
		len = send(sd, buffer, strlen(buffer), 0);
		if (len < 0) {
			printf("send error!\n");
			exit(0);
		}
	}
	return;
}

void *start_receive(void *arg)
{
	ssize_t len;
	char buffer[BUF_SIZE];

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
		len = recv(sd, buffer, BUF_SIZE, 0);
		if (len < 0) {
			printf("recieve error!\n");
			exit(0);
		}
		if(len == 0)
		{
			printf("server disconnected!\nexiting...\n");
			exit(0);
		}

		buffer[len] = '\0';
		if(strcmp(buffer, CMD_EXIT) == 0)
		{
			printf("receive exit signal form server\nexiting...\n");
			//pthread_exit(NULL);
			pthread_mutex_lock(&contrl_mutex);
			is_exit_status = 1;
			pthread_cond_signal(&contrl_cond);
			pthread_mutex_unlock(&contrl_mutex);
		}
		printf("receive[%d]:%s\n", (int)len, buffer);
	}
	pthread_exit(NULL);
}

