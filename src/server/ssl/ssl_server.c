#include "ssl_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../tcp/tcp_server_priv.h"
#include "util/logs.h"
#include "util/error.h"

typedef struct SSLServer {
    TCPServer tcp_server;
    SSL_CTX*  ctx;
} SSLServer;

static int on_error(const char* str, size_t sz, void* data)
{
    (void) data; (void) sz;
    ERR("ssl server: %s", str);
    return 0;
}


static int ssl_server_initialize(SSLServer* ssl_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key)
{
#define CHECK(v) { if (!v) { ERR_print_errors_cb(on_error, NULL); return -1; } }
    memset(ssl_server, 0, sizeof(*ssl_server));
    tcp_server_initialize((TCPServer *) ssl_server, port, open_to_world, create_session_cb, n_threads);

    SSL_METHOD const* method = TLS_server_method();
    ssl_server->ctx = SSL_CTX_new(method); CHECK(ssl_server->ctx)

    // public key
    BIO *bio = BIO_new_mem_buf(key->public_key, -1); CHECK(bio)
    X509 *cert = PEM_read_bio_X509(bio, NULL, 0, NULL); CHECK(cert)
    BIO_free(bio);
    if (SSL_CTX_use_certificate(ssl_server->ctx, cert) <= 0) {
        ERR_print_errors_cb(on_error, NULL);
        return -1;
    }

    // private key
    bio = BIO_new_mem_buf(key->private_key, -1); CHECK(bio)
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL); CHECK(pkey)
    BIO_free(bio);
    if (SSL_CTX_use_PrivateKey(ssl_server->ctx, pkey) <= 0 ) {
        ERR_print_errors_cb(on_error, NULL);
        return -1;
    }
    
    return 0;
#undef CHECK
}

static void ssl_server_finalize(SSLServer* ssl_server)
{
    SSL_CTX_free(ssl_server->ctx);
    tcp_server_finalize((TCPServer *) ssl_server);
}

SSLServer* ssl_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key)
{
    SSLServer* ssl_server = malloc(sizeof(SSLServer));
    if (ssl_server == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    if (ssl_server_initialize(ssl_server, port, open_to_world, create_session_cb, n_threads, key) < 0)
        exit(NON_RECOVERABLE_ERROR);
    return ssl_server;
}

void ssl_server_destroy(SSLServer* ssl_server)
{
    ssl_server_finalize(ssl_server);
    free(ssl_server);
}
