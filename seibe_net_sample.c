
#include <stdio.h>
#include "seibe_net.h"

void onAccept(int fd, int *res);
void onMessage(int fd, int *res);
void onConnect(int fd, int *res);
void onMessage2(int fd, int *res);
int s;

int main(int argc, char *argv[])
{
	SeibeServer server;
	SeibeClient client;

	switch (argc)
	{
		case 3:
			s = 1;
			client = SeibeClient_create( argv[1], argv[2] );
			SeibeClient_connect(client, onConnect, onMessage2);
			SeibeClient_destroy(client);
			break;
		case 2:
			server = SeibeServer_create( argv[1] );
			SeibeServer_listen(server, 10, onAccept, onMessage);
			SeibeServer_destroy(server);
			break;
		default:
			fprintf(stderr, "(server) %s port\n", argv[0]);
			fprintf(stderr, "(client) %s port hostname\n", argv[0]);
			return -1;
	}

	return 0;
}

void onAccept(int fd, int *res)
{
	printf("(server) on accept! (%d)\n", fd);
}

void onMessage(int fd, int *res)
{
	int n;
	char buf[256];
	printf("(server) on message! (%d)\n", fd);

	n = read(fd, buf, 256);
	write(1, buf, n);
	
	if (n == 0) {
		printf("(server) on close! (%d)\n", fd);
		*res = -1;
	}
}

void onConnect(int fd, int *res)
{
	printf("(client) on connect! (%d)\n", fd);
	s = fd;
}

void onMessage2(int fd, int *res)
{
	int n;
	char buf[256];
	printf("(client) on message! (%d)\n", fd);

	n = read(fd, buf, 256);
	write(fd == 0 ? s : 1, buf, n);
	
	if (n == 0) {
		printf("(client) on close! (%d)\n", fd);
		*res = -1;
	}
}
