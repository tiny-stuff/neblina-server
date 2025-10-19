#ifndef SSLCLIENT_H_
#define SSLCLIENT_H_

#include "tcpclient.h"

typedef struct SSLClient SSLClient;

void ssl_subsystem_init();
void ssl_subsystem_finalize();

SSLClient* sslclient_create(const char* host, int port);
void       sslclient_destroy(SSLClient* sslclient);

#endif
