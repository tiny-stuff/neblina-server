#ifndef NEBLINA_SERVER_SERVER_PRIV_H
#define NEBLINA_SERVER_SERVER_PRIV_H

#include <uthash/uthash.h>

#include "server.h"
#include "poller.h"

typedef struct SessionPool SessionPool;

typedef struct SessionHash {
    SOCKET         fd;
    Session*       session;
    UT_hash_handle hh;
} SessionHash;

typedef struct ServerVTable {
    int    (*recv)(SOCKET fd, uint8_t** data);
    int    (*send)(SOCKET fd, uint8_t const* data, size_t data_sz);

    SOCKET (*accept_new_connection)(Server* server);
} ServerVTable;

typedef struct Server {
    ServerVTable const* vt;
    SOCKET              fd;
    CreateSessionF      create_session_cb;
    void*               session_data;
    SessionPool*        spool;
    Poller*             poller;
    SessionHash*        session_hash;
} Server;

void server_initialize(Server* server, SOCKET fd, CreateSessionF create_session_cb, void* session_data, size_t n_threads);
void server_finalize(Server* server);

#endif //NEBLINA_SERVER_SERVER_PRIV_H
