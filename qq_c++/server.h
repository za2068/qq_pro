#ifndef __SERVER__
#define __SERVER__



#include <iostream>
#include <sys/socket.h>

#define	BUF_SIZE	1024
#define	IP_SIZE	15

class qqServer
{

public:
	qqServer();
	~qqServer();
private:
	struct sockaddr_in m_server_addr;
	int m_sd;//socket 


	char ip[IP_SIZE];
	char buffer[BUF_SIZE];

	int initServer();
	int initSocket;




}

#endif
