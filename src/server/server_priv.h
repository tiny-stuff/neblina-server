#ifndef NEBLINA_SERVER_SERVER_PRIV_H
#define NEBLINA_SERVER_SERVER_PRIV_H

#include "server.h"

typedef struct CPool CPool;

typedef struct ServerVTable {
    ServerRecvF    recv;
    ServerSendF    send;
    ServerIterateF iterate;
    ServerFreeF    free;
} ServerVTable;


typedef struct Server {
    ServerVTable const* vt;
    CreateSessionF      create_session;
    CPool*              cpool;
    SOCKET              fd;
} Server;

#endif //NEBLINA_SERVER_SERVER_PRIV_H
