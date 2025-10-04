#include "server.h"

#include <stdlib.h>

#include "connection.h"

typedef struct Server {
    ServerRecvF recv;
    ServerSendF send;
} Server;

Server* server_create(ServerRecvF recv, ServerSendF send)
{
    Server* server = calloc(1, sizeof(Server));
    server->recv = recv;
    server->send = send;
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

    // receive data
    uint8_t* recv_buf;
    r = server->recv(connection_socket_fd(connection), &recv_buf);
    if (r < 0)
        return r;
    if (r > 0) {
        connection_add_to_recv_buffer(connection, recv_buf, r);
        // TODO - call session
        free(recv_buf);
    }

    // done - clear buffers
    connection_clear_buffers(connection);

    return 0;
}
