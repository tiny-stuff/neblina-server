#include "tcpclient.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcpclient_priv.h"

void tcpclient_initialize(TCPClient* tcpclient)
{
    memset(tcpclient, 0, sizeof(*tcpclient));
    // TODO: initialize fields here
}

void tcpclient_finalize(TCPClient* tcpclient)
{
    // TODO: finalize fields here
}

TCPClient* tcpclient_create(const char* host, int port)
{
    TCPClient* tcpclient = malloc(sizeof(TCPClient));
    if (tcpclient == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    tcpclient_initialize(tcpclient);
    return tcpclient;
}

void tcpclient_destroy(TCPClient* tcpclient)
{
    tcpclient_finalize(tcpclient);
    free(tcpclient);
}

int tcpclient_send(TCPClient* t, uint8_t* data, size_t sz)
{
    // TODO
    return 0;
}

int tcpclient_send_text(TCPClient* t, const char* data)
{
    // TODO
    return 0;
}

int tcpclient_recv(TCPClient* t, uint8_t** data)
{
    // TODO
    return 0;
}

int tcpclient_recv_block(TCPClient* t, uint8_t* data, size_t sz)
{
    // TODO
    return 0;
}
