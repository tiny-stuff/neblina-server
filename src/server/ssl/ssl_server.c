#include "ssl_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../tcp/tcp_server_priv.h"

typedef struct SSLServer {
    TCPServer tcp_server;
    SSL_CTX*  ctx;
} SSLServer;

static void ssl_server_initialize(SSLServer* ssl_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key)
{
    memset(ssl_server, 0, sizeof(*ssl_server));
    tcp_server_initialize((TCPServer *) ssl_server, port, open_to_world, create_session_cb, n_threads);

    SSL_METHOD const* method = TLS_server_method();
    ssl_server->ctx = SSL_CTX_new(method);
    if (!ssl_server->ctx) {
        ERR_print_errors_fp(stderr);
        throw NonRecoverableException("Could not initialize OpenSSL.");
    }

    if (SSL_CTX_use_certificate_file(ssl_server->ctx, key->public_key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        throw NonRecoverableException("Could not load public key.");
    }

    if (SSL_CTX_use_PrivateKey_file(ssl_server->ctx, key->private_key, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        throw NonRecoverableException("Could not load private key.");
    }
}

static void ssl_server_finalize(SSLServer* ssl_server)
{
    tcp_server_finalize((TCPServer *) ssl_server);
}

SSLServer* ssl_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key)
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
