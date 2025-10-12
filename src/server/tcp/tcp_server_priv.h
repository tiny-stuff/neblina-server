#ifndef NEBLINA_SERVER_TCP_SERVER_PRIV_H
#define NEBLINA_SERVER_TCP_SERVER_PRIV_H

#include "../server_priv.h"

typedef struct TCPServer {
    Server  server;
    int     port;
    bool    open_to_world;
} TCPServer;

int  tcp_server_initialize(TCPServer* tcp_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads);
void tcp_server_finalize(TCPServer* tcp_server);

#endif //NEBLINA_SERVER_TCP_SERVER_PRIV_H
