#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include "client.h"

typedef struct TCPClient TCPClient;

TCPClient* tcpclient_create(const char* host, int port);
void       tcpclient_destroy(TCPClient* tcpclient);

#endif
