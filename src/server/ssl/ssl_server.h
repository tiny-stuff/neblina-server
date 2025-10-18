#ifndef SSLSERVER_H_
#define SSLSERVER_H_

#include "server/tcp/tcp_server.h"

typedef struct SSLServer SSLServer;

typedef struct SSLKey {
    const char* public_key;
    const char* private_key;
} SSLKey;

SSLServer* ssl_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads, SSLKey const* key);
void       ssl_server_destroy(SSLServer* sslserver);

#endif
