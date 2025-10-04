#include "server.h"

#include <stdlib.h>

#include "connection.h"
#include "service/session.h"
#include "cpool/cpool.h"

typedef struct Server {
    ServerRecvF    recv;
    ServerSendF    send;
    CreateSessionF create_session;
    CPool*         cpool;
} Server;

Server* server_create(ServerRecvF recv, ServerSendF send, CreateSessionF create_session, size_t n_threads)
{
    Server* server = calloc(1, sizeof(Server));
    server->recv = recv;
    server->send = send;
    server->create_session = create_session;
    server->cpool = cpool_create(n_threads, server);
    return server;
}

void server_destroy(Server* server)
{
    free(server);
}

int server_flush_connection(Server* server, Connection* connection)
{
    // send pending data
    size_t sz;
    uint8_t const* data_to_send = connection_send_buffer(connection, &sz);
    int r = server->send(connection_socket_fd(connection), data_to_send, sz);
    if (r < 0)
        return r;
    connection_clear_send_buffer(connection);

    // receive data
    uint8_t* recv_buf;
    r = server->recv(connection_socket_fd(connection), &recv_buf);
    if (r < 0)
        return r;
    if (r > 0) {
        connection_add_to_recv_buffer(connection, recv_buf, r);
        session_on_recv(connection_session(connection), connection);  // TODO - check for errors
        free(recv_buf);
    }

    return 0;
}

int server_iterate(Server* server, size_t timeout_ms)
{
    // TODO - ???
    return 0;
}
