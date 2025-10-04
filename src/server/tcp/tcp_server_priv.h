#ifndef NEBLINA_SERVER_TCP_SERVER_PRIV_H
#define NEBLINA_SERVER_TCP_SERVER_PRIV_H

#include "../server_priv.h"

typedef struct TCPServer {
    Server server;
    int    port;
    bool   open_to_world;
} TCPServer;

#endif //NEBLINA_SERVER_TCP_SERVER_PRIV_H
