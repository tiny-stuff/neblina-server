#ifndef NEBLINA_SERVER_SERVER_H
#define NEBLINA_SERVER_SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "connection.h"

#define RECV_BUF_SZ (16 * 1024)

typedef struct Server Server;

typedef int(*ServerRecvF)(SOCKET fd, uint8_t** data);
typedef int(*ServerSendF)(SOCKET fd, uint8_t const* data, size_t data_sz);

Server* server_create(ServerRecvF recv, ServerSendF send);
void    server_destroy(Server* server);

int server_flush_connection(Server* server, Connection* connection);

#endif //NEBLINA_SERVER_SERVER_H
