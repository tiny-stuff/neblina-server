#ifndef NEBLINA_SERVER_TCP_SERVER_H
#define NEBLINA_SERVER_TCP_SERVER_H

#include <stdbool.h>

#include "../server.h"

Server* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads);

#endif //NEBLINA_SERVER_TCP_SERVER_H
