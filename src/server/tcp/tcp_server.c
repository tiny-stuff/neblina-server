#include "tcp_server.h"

#include <stdlib.h>

#include "tcp_server_priv.h"

static int tcp_server_recv(SOCKET fd, uint8_t** data)
{
    return 0;
}

static int tcp_server_send(SOCKET fd, uint8_t const* data, size_t data_sz)
{
    return 0;
}

static void tcp_server_free(Server* server)
{
    free(server);
}

Server* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session, size_t n_threads)
{
    TCPServer* tcp_server = calloc(1, sizeof(TCPServer));
    server_create(tcp_server_recv, tcp_server_send, tcp_server_free, create_session, n_threads);
    tcp_server->port = port;
    tcp_server->open_to_world = open_to_world;
    return (Server *) tcp_server;
}