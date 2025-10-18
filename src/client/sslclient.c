#include "sslclient.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "util/error.h"
#include "tcpclient_priv.h"
#include "os.h"

static int on_error(const char* str, size_t sz, void* data)
{
    (void) data; (void) sz;
    ERR("ssl client: %s", str);
    return 0;
}

//-------------------------------
// SSL Subsystem
//-------------------------------

static SSL_CTX* ctx = NULL;

void ssl_subsystem_init(void)
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        ERR_print_errors_cb(on_error, NULL);
        exit(NON_RECOVERABLE_ERROR);
    }

    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        ERR("Failed to load system CA store");
        exit(NON_RECOVERABLE_ERROR);
    }
}

void ssl_subsystem_finalize(void)
{
    if (ctx)
        SSL_CTX_free(ctx);
}

//-------------------------------
// SSL Connection
//-------------------------------

typedef struct SSLClient {
    TCPClient base;
    SSL*      ssl;
} SSLClient;

static ssize_t recv_(Client* client, uint8_t* data, size_t sz)
{
    SSLClient* sslclient = (SSLClient *) client;
    return SSL_read(sslclient->ssl, data, (int) sz);
}

static ssize_t send_(Client* client, uint8_t const* data, size_t sz)
{
    SSLClient* sslclient = (SSLClient *) client;
    return SSL_write(sslclient->ssl, data, (int) sz);
}

static SSL* wrap_socket(SSLClient* sslclient)
{
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sslclient->base.base.fd);

    // handshake
    while (1) {
        int ret = SSL_connect(ssl);
        if (ret == 1) {
            DBG("sslclient: handshake complete!\n");
            break;
        }

        int err = SSL_get_error(ssl, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            os_sleep_ms(1);
            continue;
        } else {
            ERR("Could not open a SSL connection");
            ERR_print_errors_fp(stderr);
            ERR_print_errors_cb(on_error, NULL);
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            return NULL;
        }
    }

    if (SSL_connect(ssl) <= 0) {
        ERR("SSL client: handshake failed");
        ERR_print_errors_cb(on_error, NULL);
        SSL_free(ssl);
        return NULL;
    }

    // verify certificate
    /*
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        ERR("Certificate verification failed!");
        ERR_print_errors_cb(on_error, NULL);
        SSL_free(ssl);
        return NULL;
    }
     */

    return ssl;

}

static int sslclient_initialize(SSLClient* sslclient, const char* host, int port)
{
    if (ctx == NULL) {
        fprintf(stderr, "SSL subsystem was not initialized!\n");
        exit(NON_RECOVERABLE_ERROR);
    }
    memset(sslclient, 0, sizeof(*sslclient));
    tcpclient_initialize(&sslclient->base, host, port);
    sslclient->ssl = wrap_socket(sslclient);
    if (sslclient->ssl == NULL)
        return -1;
    sslclient->base.base.vt.recv = recv_;
    sslclient->base.base.vt.send = send_;
    return 0;
}

static void sslclient_finalize(SSLClient* sslclient)
{
    tcpclient_finalize(&sslclient->base);
    if (sslclient->ssl)
        SSL_free(sslclient->ssl);
}

SSLClient* sslclient_create(const char* host, int port)
{
    SSLClient* sslclient = malloc(sizeof(SSLClient));
    if (sslclient == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    if (sslclient_initialize(sslclient, host, port) != 0) {
        sslclient_finalize(sslclient);
        return NULL;
    }
    return sslclient;
}

void sslclient_destroy(SSLClient* sslclient)
{
    sslclient_finalize(sslclient);
    free(sslclient);
}
