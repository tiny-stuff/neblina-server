#ifndef NEBLINA_SERVER_SERVER_PRIV_H
#define NEBLINA_SERVER_SERVER_PRIV_H

#include "server.h"

typedef struct CPool CPool;

typedef struct Server {
    ServerRecvF    recv;
    ServerSendF    send;
    ServerFreeF    free;
    CreateSessionF create_session;
    CPool*         cpool;
} Server;

#endif //NEBLINA_SERVER_SERVER_PRIV_H
