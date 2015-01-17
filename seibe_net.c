#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "seibe_list.h"
#include "seibe_net.h"

typedef struct seibeserver {
	int port;
	struct addrinfo *addrinfo;
} _SeibeServer;

typedef struct seibeclient {
	int port;
	struct addrinfo *addrinfo;
} _SeibeClient;


SeibeServer SeibeServer_create(const char *port)
{
	SeibeServer server;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	
	if (getaddrinfo(NULL, port, &hints, &res) != 0) return NULL;

	server = (SeibeServer)malloc(sizeof(_SeibeServer));
	server->port = atoi(port);
	server->addrinfo = res;

	return server;
}

void SeibeServer_destroy(SeibeServer server)
{
	if (server == NULL) return;

	freeaddrinfo(server->addrinfo);
	free(server);
}

int SeibeServer_listen(SeibeServer server, const int backlog, SeibeNetFunc onConnect, SeibeNetFunc onMessage)
{
	SeibeList list;
	struct addrinfo *info;
	fd_set rfd;
	int s0[SEIBE_SERVER_SOCK_MAX];
	int i, s, n, smax, slen, clilen, loop, res, *fd;

	if (server == NULL) return -1;

	// 複数の接続経路を確保する
	i = 0;
	for (info = server->addrinfo; info != NULL && i < SEIBE_SERVER_SOCK_MAX; info = info->ai_next)
	{
		// ソケットを作成
		if ((s0[i] = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
			continue;
		}

		// 接続を待ち受け
		if (bind(s0[i], info->ai_addr, info->ai_addrlen) == -1 || listen(s0[i], backlog) == -1) {
			close(s0[i]);
			s0[i] = -1;
			continue;
		}

		++i;
	}
	slen = i;

	// ループ
	list = SeibeList_create();
	clilen = 0;
	smax = -1;
	loop = 1;
	void SeibeServer_listen_each1(void *data, int *code) {
		fd = (int *)data;
		FD_SET(*fd, &rfd);
		smax = *fd > smax ? *fd : smax;
	}
	void SeibeServer_listen_each2(void *data, int *code) {
		fd = (int *)data;
		if (FD_ISSET(*fd, &rfd)) {
			onMessage(*fd, &res);

			switch(res) {
				case -2:
					*code = -2;
					loop = 0;
					break;

				case -1:
					*code = -1;
					close(*fd);
					break;

				default:
					break;
			}
		}
	}
	while (loop) {
		// 入力の多重化
		FD_ZERO(&rfd);
		for (i = 0; i < slen; ++i) {
			FD_SET(s0[i], &rfd);
			smax = s0[i] > smax ? s0[i] : smax;
		}
		SeibeList_forEach(list, SeibeServer_listen_each1);
		if (select(smax + 1, &rfd, NULL, NULL, NULL) == -1) return -1;

		// 接続要求の処理
		for (i = 0; i < slen; ++i) {
			if (FD_ISSET(s0[i], &rfd)) {
				// 接続許可
				s = accept(s0[i], NULL, NULL);
				if (s != -1) {
					++clilen;
					fd = (int *)malloc(sizeof(int));
					*fd = s;
					SeibeList_push(list, fd);
					if (onConnect != NULL) {
						res = 0;
						onConnect(s, &res);
					}
				}
			}
		}

		// 送受信データの処理
		if (onMessage != NULL) {
			SeibeList_forEach(list, SeibeServer_listen_each2);
		}
	}

	return 0;
}


SeibeClient SeibeClient_create(const char *port, const char *hostname)
{
	SeibeClient client;
	struct addrinfo hints, *res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;

	if (getaddrinfo(hostname, port, &hints, &res) != 0) return NULL;

	client = (SeibeClient)malloc(sizeof(_SeibeClient));
	client->port = atoi(port);
	client->addrinfo = res;

	return client;
}

void SeibeClient_destroy(SeibeClient client)
{
	if (client == NULL) return;

	freeaddrinfo(client->addrinfo);
	free(client);
}

int SeibeClient_connect(SeibeClient client, SeibeNetFunc onConnect, SeibeNetFunc onMessage)
{
	struct addrinfo *info;
	fd_set rfd;
	int i, s, res, loop;

	if (client == NULL) return -1;

	// 順に接続を試みる
	for (info = client->addrinfo; info != NULL; info = info->ai_next)
	{
		// ソケットを作成
		if ((s = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) continue;

		// サーバーに接続する
		res = 0;
		if (connect(s, info->ai_addr, info->ai_addrlen) != 0) return -1;
		if (onConnect) onConnect(s, &res);

		// ループ
		loop = 1;
		while (loop) {
			// 入力の多重化
			FD_ZERO(&rfd);
			FD_SET(0, &rfd);
			FD_SET(s, &rfd);
			if (select(s+1, &rfd, NULL, NULL, NULL) == -1) return -1;

			if (FD_ISSET(0, &rfd)) {
				res = 0;
				if (onMessage) onMessage(0, &res);
				if (res < 0) loop = 0;
			}
			if (FD_ISSET(s, &rfd)) {
				res = 0;
				if (onMessage) onMessage(s, &res);
				if (res < 0) loop = 0;
			}
		}

		// ソケットを閉じる
		close(s);
		break;
	}

	return 0;
}
