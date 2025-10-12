#ifndef NEBLINA_SERVER_TCP_SERVER_H
#define NEBLINA_SERVER_TCP_SERVER_H

#include <stdbool.h>

#include "../server.h"

typedef struct TCPServer TCPServer;

TCPServer* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads);
void       tcp_server_destroy(TCPServer* tcp_server);

#endif //NEBLINA_SERVER_TCP_SERVER_H
