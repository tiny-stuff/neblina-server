#include "server.h"
#include "server_priv.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "commbuf.h"
#include "service/session.h"
#include "spool/spool.h"
#include "util/logs.h"
#include "util/alloc.h"

#define MAX_EVENTS 64

extern volatile bool termination_requested;

void server_initialize(Server* server, SOCKET fd, CreateSessionF create_session_cb, void* session_data, size_t n_threads)
{
    server->create_session_cb = create_session_cb;
    server->session_data = session_data;
    server->spool = spool_create(n_threads, server);
    server->fd = fd;
    server->poller = poller_create(fd);
    server->session_hash = NULL;
}

static bool server_is_session_active(Server* server, Session* session)
{
    SessionHash* conn_hash;
    HASH_FIND_INT(server->session_hash, &session->fd, conn_hash);
    return conn_hash != NULL;
}

int server_process_session(Server* server, Session* session)
{
    if (!server_is_session_active(server, session))
        return 0;

    // receive data
    uint8_t* recv_buf;
    int r = server->vt->recv(server, session->fd, &recv_buf);
    if (r < 0) {
        ERR("Error receiving data from socket %d: %s", session->fd, strerror(errno));
        return r;
    }
    if (r > 0) {
        commbuf_add_to_recv_buffer(session->connection, recv_buf, r);
        int rr = session_on_recv(session);
        free(recv_buf);
        if (rr < 0) {
            ERR("Error on session connected to socket %d: %s (possible description)", session->fd, strerror(errno));
            close_socket(session->fd);
        }
    }

    // send pending data
    size_t sz;
    uint8_t const* data_to_send = commbuf_send_buffer(session->connection, &sz);
    r = server->vt->send(server, session->fd, data_to_send, sz);
    if (r < 0) {
        ERR("Error sending data to socket %d: %s", session->fd, strerror(errno));
        return r;
    }
    commbuf_clear_send_buffer(session->connection);

    return 0;
}

static int handle_new_connection(Server* server)
{
    // accept new connection
    SOCKET client_fd = server->vt->accept_new_connection(server);
    if (client_fd == INVALID_SOCKET) {
        ERR("error accepting a new connection: %s", strerror(errno));
        close_socket(client_fd);
        return -1;
    }

    // add socket to poller (allows polling for new data)
    poller_add_connection(server->poller, client_fd);

    // create and add session to hash
    SessionHash* conn_hash = MALLOC(sizeof *conn_hash);
    conn_hash->fd = client_fd;
    conn_hash->session = server->create_session_cb(client_fd, server->session_data);
    HASH_ADD_INT(server->session_hash, fd, conn_hash);

    // add connection to connection pool
    spool_add_session(server->spool, conn_hash->session);

    return 0;
}

static void handle_disconnect(Server* server, SOCKET client_fd)
{
    DBG("Client disconnected from socket %d", client_fd);

    if (server->vt->client_disconnected)
        server->vt->client_disconnected(server, client_fd);

    // find connection in list
    SessionHash* conn_hash;
    HASH_FIND_INT(server->session_hash, &client_fd, conn_hash);
    if (!conn_hash)
        return;

    // remove connection from connection pool
    spool_remove_session(server->spool, conn_hash->session);

    // destroy session
    session_finalize(conn_hash->session);
    free(conn_hash->session);

    // remove session from hash, and destroy it
    HASH_DEL(server->session_hash, conn_hash);
    free(conn_hash);

    // remove socket from poller
    poller_remove_connection(server->poller, client_fd);

    // close socket
    server_close_socket(client_fd);
}

static void handle_new_data(Server* server, SOCKET client_fd)
{
    SessionHash* conn_hash;
    HASH_FIND_INT(server->session_hash, &client_fd, conn_hash);
    if (conn_hash == NULL)
        return;

    spool_flush_session(server->spool, conn_hash->session);
}

int server_iterate(Server* server, size_t timeout_ms)
{
    PollerEvent events[MAX_EVENTS];
    size_t n_events = poller_wait(server->poller, events, MAX_EVENTS, timeout_ms);   // TODO - check for errors
    for (size_t i = 0; i < n_events; ++i) {
        switch (events[i].type) {
            case PT_NEW_CONNECTION:
                handle_new_connection(server);
                break;
            case PT_NEW_DATA:
                handle_new_data(server, events[i].fd);
                break;
            case PT_DISCONNECTED:
                handle_disconnect(server, events[i].fd);
                break;
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
    close_socket(fd);
}

void server_finalize(Server* server)
{
    // close all connections
    SessionHash *conn_hash, *tmp;
    HASH_ITER(hh, server->session_hash, conn_hash, tmp) {
        handle_disconnect(server, conn_hash->fd);
    }

    server_close_socket(server->fd);
    poller_destroy(server->poller);
    spool_destroy(server->spool);
    DBG("Server destroyed");
}

void server_destroy(Server* server)
{
    server_finalize(server);
    free(server);
}

