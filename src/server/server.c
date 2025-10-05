#include "server.h"
#include "server_priv.h"

#include <stdlib.h>
#include <stdio.h>

#include "connection.h"
#include "service/session.h"
#include "cpool/cpool.h"
#include "util/logs.h"

#define MAX_EVENTS 64

extern bool termination_requested;

void server_init(Server* server, SOCKET fd, CreateSessionF create_session_cb, void* session_data, size_t n_threads)
{
    server->create_session_cb = create_session_cb;
    server->session_data = session_data;
    server->cpool = cpool_create(n_threads, server);
    server->fd = fd;
    server->poller = poller_create(fd);
    server->connection_hash = NULL;
}

void server_destroy(Server* server)
{
    server_close_socket(server->fd);
    poller_destroy(server->poller);
    cpool_destroy(server->cpool);
    server->vt->free(server);
    DBG("Server destroyed");
}

int server_flush_connection(Server* server, Connection* connection)
{
    // send pending data
    size_t sz;
    uint8_t const* data_to_send = connection_send_buffer(connection, &sz);
    int r = server->vt->send(connection_socket_fd(connection), data_to_send, sz);
    if (r < 0)
        return r;
    connection_clear_send_buffer(connection);

    // receive data
    uint8_t* recv_buf;
    r = server->vt->recv(connection_socket_fd(connection), &recv_buf);
    if (r < 0)
        return r;
    if (r > 0) {
        connection_add_to_recv_buffer(connection, recv_buf, r);
        session_on_recv(connection_session(connection), connection);  // TODO - check for errors
        free(recv_buf);
    }

    return 0;
}

static void handle_new_connection(Server* server)
{
    // accept new connection
    SOCKET client_fd = server->vt->accept_new_connection(server);

    // add socket to poller (allows polling for new data)
    poller_add_connection(server->poller, client_fd);

    // create session from service
    Session* session = server->create_session_cb(server->session_data);

    // create and add connection to hash
    ConnectionHash* conn_hash = malloc(sizeof *conn_hash);
    conn_hash->fd = client_fd;
    conn_hash->connection = connection_create(client_fd, session);
    HASH_ADD_INT(server->connection_hash, fd, conn_hash);

    // add connection to connection pool
    cpool_add_connection(server->cpool, conn_hash->connection);
}

static void handle_new_data(Server* server, SOCKET client_fd)
{
    DBG("New data");
}

static void handle_disconnect(Server* server, SOCKET client_fd)
{
    DBG("Client disconnected from socket %d", client_fd);

    // TODO - remove connection + session + cpool

    // remove from poller
    poller_remove_connection(server->poller, client_fd);

    // close socket
    server_close_socket(client_fd);
}

int server_iterate(Server* server, size_t timeout_ms)
{
    PollerEvent events[MAX_EVENTS];
    size_t n_events = poller_wait(server->poller, events, MAX_EVENTS, timeout_ms);   // TODO - check for errors
    for (size_t i = 0; i < n_events; ++i) {
        switch (events[i].type) {
            case PT_NEW_CONNECTION: handle_new_connection(server); break;
            case PT_NEW_DATA:       handle_new_data(server, events[i].fd); break;
            case PT_DISCONNECTED:   handle_disconnect(server, events[i].fd); break;
        }
    }
    return 0;
}

void server_run(Server* server)
{
    while (!termination_requested)
        server_iterate(server, 50);
}

void server_close_socket(SOCKET fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}
