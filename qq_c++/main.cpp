#include <iostream>


#include "client.h"
#include "server.h"
#include "message_event.h"


int main(void)
{
	qqServer server = new qqserver();
	server.start();

	return 0;
}
