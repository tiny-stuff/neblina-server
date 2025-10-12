#include "ssl_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "../tcp/tcp_server_priv.h"
#include "util/logs.h"
#include "util/error.h"
#include "util/alloc.h"
#include "uthash/uthash.h"

static const char* ERR_PRX = "SSL server error:";

typedef struct SSL_FD_Map {
    SOCKET fd;
    SSL*   ssl;
    UT_hash_handle hh;
} SSL_FD_Map;

typedef struct SSLServer {
    TCPServer   tcp_server;
    SSL_CTX*    ctx;
    SSL_FD_Map* ssl_fd_map;
} SSLServer;

static int on_error(const char* str, size_t sz, void* data)
{
    (void) data; (void) sz;
    ERR("ssl server: %s", str);
    return 0;
}

static SOCKET ssl_accept_new_connection(Server* server)
{
    SSLServer* ssl_server = (SSLServer *) server;

    // accept new TCP connection
    SOCKET fd = tcp_accept_new_connection(server);

    // add SSL wrapper
    SSL* ssl = SSL_new(ssl_server->ctx);
    SSL_set_fd(ssl, fd);
    if (SSL_accept(ssl) <= 0) {
        ERR("Could not open a SSL connection");
        ERR_print_errors_fp(stderr);
        ERR_print_errors_cb(on_error, NULL);
        return INVALID_SOCKET;
    }

    // store the pair ssl/fd
    SSL_FD_Map* ssl_fd = CALLOC(1, sizeof(SSL_FD_Map));
    ssl_fd->fd = fd;
    ssl_fd->ssl = ssl;
    HASH_ADD_INT(ssl_server->ssl_fd_map, fd, ssl_fd);

    return fd;
}

static SSL* find_ssl(Server* server, SOCKET fd)
{
    SSLServer* ssl_server = (SSLServer *) server;
    SSL_FD_Map* found;
    HASH_FIND_INT(ssl_server->ssl_fd_map, &fd, found);
    if (!found)
        return NULL;
    return found->ssl;
}

static int ssl_server_recv(Server* server, SOCKET fd, uint8_t** data)
{
    SSL* ssl = find_ssl(server, fd);
    if (ssl) {
        *data = MALLOC(RECV_BUF_SZ);
        ssize_t r = SSL_read(ssl, *data, RECV_BUF_SZ);
        if (r < 0) {
            ERR_print_errors_cb(on_error, NULL);
            ERR("%s SSL_read error: %s", ERR_PRX, strerror(errno));
        }
        return (int) r;
    }
    return -1;
}

static int ssl_server_send(Server* server, SOCKET fd, uint8_t const* data, size_t data_sz)
{
    SSL* ssl = find_ssl(server, fd);
    if (ssl) {
        ssize_t r = SSL_write(ssl, data, (int) data_sz);
        if (r < 0)
            ERR("%s send error: %s", ERR_PRX, strerror(errno));
        return (int) r;
    }
    return -1;
}

static int ssl_server_initialize(SSLServer* ssl_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key)
{
    memset(ssl_server, 0, sizeof(*ssl_server));

    // initialize parent
    if (tcp_server_initialize((TCPServer *) ssl_server, port, open_to_world, create_session_cb, n_threads) < 0)
        return -1;

    // setup vtable
    static const ServerVTable ssl_vtable = {
            .recv = ssl_server_recv,
            .send = ssl_server_send,
            .accept_new_connection = ssl_accept_new_connection,
    };
    ssl_server->tcp_server.server.vt = &ssl_vtable;

#define CHECK(v) { if ((v) == NULL) { ERR_print_errors_cb(on_error, NULL); return -1; } }
    // initialize openssl
    SSL_METHOD const* method = TLS_server_method();
    ssl_server->ctx = SSL_CTX_new(method); CHECK(ssl_server->ctx)

    // public key
    BIO *bio = BIO_new_mem_buf(key->public_key, -1); CHECK(bio)
    X509 *cert = PEM_read_bio_X509(bio, NULL, 0, NULL); CHECK(cert)
    BIO_free(bio);
    if (SSL_CTX_use_certificate(ssl_server->ctx, cert) <= 0) {
        X509_free(cert);
        ERR_print_errors_cb(on_error, NULL);
        return -1;
    }
    X509_free(cert);

    // private key
    bio = BIO_new_mem_buf(key->private_key, -1); CHECK(bio)
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL); CHECK(pkey)
    BIO_free(bio);
    if (SSL_CTX_use_PrivateKey(ssl_server->ctx, pkey) <= 0 ) {
        EVP_PKEY_free(pkey);
        ERR_print_errors_cb(on_error, NULL);
        return -1;
    }
    EVP_PKEY_free(pkey);

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
