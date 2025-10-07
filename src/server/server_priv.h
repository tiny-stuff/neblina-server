#ifndef NEBLINA_SERVER_SERVER_PRIV_H
#define NEBLINA_SERVER_SERVER_PRIV_H

#include "server.h"
#include "poller.h"
#include "uthash/uthash.h"

typedef struct CPool CPool;

typedef struct ConnectionMap {
    int            fd;
    Connection*    connection;
    UT_hash_handle hh;
} ConnectionHash;

typedef struct ServerVTable {
    void   (*free)(Server* server);

    int    (*recv)(SOCKET fd, uint8_t** data);
    int    (*send)(SOCKET fd, uint8_t const* data, size_t data_sz);

    SOCKET (*accept_new_connection)(Server* server);
} ServerVTable;

typedef struct Server {
    ServerVTable const* vt;
    SOCKET              fd;
    CreateSessionF      create_session_cb;
    void*               session_data;
    CPool*              cpool;
    Poller*             poller;
    ConnectionHash*     connection_hash;
} Server;

#endif //NEBLINA_SERVER_SERVER_PRIV_H
