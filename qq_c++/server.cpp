#include "server.h"

#define	MY_PORT	6788

qqServer::qqServer()
{
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_server_addr.sin_port = htons(MY_PORT);

	
	if((m_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error!\n");
		exit(0);
	}

	if (bind(m_sd, (struct sockaddr*)&server_addr, 
				sizeof(server_addr)) < 0) {
		printf("bind error!\n");
		exit(0);
	}

	if (listen(m_sd, 128) < 0) {
		printf("listen error!\n");
		exit(0);
	}

}

qqServer::~qqServer()
{
}



