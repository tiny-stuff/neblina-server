#ifndef TCPCLIENT_PRIV_H_
#define TCPCLIENT_PRIV_H_

#include "socket.h"

typedef struct TCPClientVTable {
    int (*recv)(SOCKET fd, uint8_t* data, size_t sz);
    int (*send)(SOCKET fd, uint8_t const* data, size_t sz);
} TCPClientVTable;

typedef struct TCPClient {
    TCPClientVTable vt;
    SOCKET fd;
} TCPClient;

int  tcpclient_initialize(TCPClient* tcpclient, const char* host, int port);
void tcpclient_finalize(TCPClient* tcpclient);

#endif
