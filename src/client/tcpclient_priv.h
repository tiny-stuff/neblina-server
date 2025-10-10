#ifndef TCPCLIENT_PRIV_H_
#define TCPCLIENT_PRIV_H_

typedef struct TCPClient {
    // TODO: add fields here
} TCPClient;

typedef struct TCPClientVTable {
    // TODO: add methods here
} TCPClientVTable;

void tcpclient_initialize(TCPClient* tcpclient);
void tcpclient_finalize(TCPClient* tcpclient);

#endif
