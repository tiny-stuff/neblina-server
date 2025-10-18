#ifndef TCPCLIENT_PRIV_H_
#define TCPCLIENT_PRIV_H_

#include "client_priv.h"

typedef struct TCPClient {
    Client base;
} TCPClient;

void tcpclient_initialize(TCPClient* tcpclient, const char* host, int port);
void tcpclient_finalize(TCPClient* tcpclient);

#endif
