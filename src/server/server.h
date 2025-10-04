#ifndef NEBLINA_SERVER_SERVER_H
#define NEBLINA_SERVER_SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "connection.h"

#define RECV_BUF_SZ (16 * 1024)

#define SINGLE_THREADED 0

typedef struct Server Server;
typedef struct Session Session;

typedef int(*ServerRecvF)(SOCKET fd, uint8_t** data);
typedef int(*ServerSendF)(SOCKET fd, uint8_t const* data, size_t data_sz);
typedef void(*ServerFreeF)(Server* server);
typedef void(*ServerIterateF)(Server* server);

typedef Session*(*CreateSessionF)(void* data);

Server* server_init(CreateSessionF create_session, size_t n_threads);
void    server_finalize(Server* server);

int     server_iterate(Server* server, size_t timeout_ms);
int     server_flush_connection(Server* server, Connection* connection);

void    server_run(Server* server);

#endif //NEBLINA_SERVER_SERVER_H
