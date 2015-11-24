/*
 *author: justaipanda
 *create time:2012/09/03 10:47:35
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_PORT 6788
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE	1024

int main() {

	int ip;
	inet_pton(AF_INET, SERVER_IP, &ip);

	//set server socket address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = ip;
	server_addr.sin_port = htons(SERVER_PORT);

	int sd;
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
	while(1)
	{
		printf("input:");
		scanf("%s", buffer);
		int len = send(sd, buffer, strlen(buffer), 0);
		if (len < 0) {
			printf("send error!\n");
			exit(0);
		}

		len = recv(sd, buffer, BUF_SIZE, 0);
		if (len < 0) {
			printf("recieve error!\n");
			exit(0);
		}

		buffer[len] = '\0';
		printf("receive[%d]:%s\n", (int)len, buffer);
	}

	return 0;
}
