#include "ssl_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../tcp/tcp_server_priv.h"

typedef struct SSLServer {
    TCPServer tcp_server;
} SSLServer;

static void ssl_server_initialize(SSLServer* ssl_server)
{
    memset(ssl_server, 0, sizeof(*ssl_server));
    // TODO: initialize fields here
}

static void ssl_server_finalize(SSLServer* ssl_server)
{
    // TODO: finalize fields here
}

SSLServer* ssl_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads)
{
    SSLServer* ssl_server = malloc(sizeof(SSLServer));
    if (ssl_server == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    ssl_server_initialize(ssl_server);
    return ssl_server;
}

void ssl_server_destroy(SSLServer* ssl_server)
{
    ssl_server_finalize(ssl_server);
    free(ssl_server);
}
