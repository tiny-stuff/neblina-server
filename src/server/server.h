#ifndef NEBLINA_SERVER_SERVER_H
#define NEBLINA_SERVER_SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "commbuf.h"

#define RECV_BUF_SZ (16 * 1024)

#define SINGLE_THREADED 0

typedef struct Server Server;
typedef struct Session Session;

typedef Session*(*CreateSessionF)(SOCKET fd, void* data);

void    server_init(Server* server, SOCKET fd, CreateSessionF create_session_cb, void* session_data, size_t n_threads);
void    server_destroy(Server* server);

int     server_iterate(Server* server, size_t timeout_ms);
int     server_process_session(Server* server, Session* session);

void    server_run(Server* server);
void    server_close_socket(SOCKET fd);

#endif //NEBLINA_SERVER_SERVER_H
