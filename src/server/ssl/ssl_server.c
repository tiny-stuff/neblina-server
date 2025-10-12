#include "ssl_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../tcp/tcp_server_priv.h"

typedef struct SSLServer {
    TCPServer tcp_server;
} SSLServer;

static void ssl_server_initialize(SSLServer* ssl_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads)
{
    memset(ssl_server, 0, sizeof(*ssl_server));
    tcp_server_initialize((TCPServer *) ssl_server, port, open_to_world, create_session_cb, n_threads);
}

static void ssl_server_finalize(SSLServer* ssl_server)
{
    tcp_server_finalize((TCPServer *) ssl_server);
}

SSLServer* ssl_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads)
{
    SSLServer* ssl_server = malloc(sizeof(SSLServer));
    if (ssl_server == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    ssl_server_initialize(ssl_server, port, open_to_world, create_session_cb, n_threads);
    return ssl_server;
}

void ssl_server_destroy(SSLServer* ssl_server)
{
    ssl_server_finalize(ssl_server);
    free(ssl_server);
}
