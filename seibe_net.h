#ifndef _INCLUDE_SEIBE_NET_H_
#define _INCLUDE_SEIBE_NET_H_

#define SEIBE_SERVER_SOCK_MAX 10

typedef struct seibeserver * SeibeServer;
typedef struct seibeclient * SeibeClient;
typedef void (*SeibeNetFunc)(const int fd, int *res);

SeibeServer SeibeServer_create(const char *port);
void SeibeServer_destroy(SeibeServer server);
int SeibeServer_listen(SeibeServer server, const int backlog, SeibeNetFunc onConnect, SeibeNetFunc onMessage);

SeibeClient SeibeClient_create(const char *port, const char *hostname);
void SeibeClient_destroy(SeibeClient client);
int SeibeClient_connect(SeibeClient client, SeibeNetFunc onConnect, SeibeNetFunc onMessage);

#endif
