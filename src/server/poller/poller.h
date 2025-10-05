#ifndef NEBLINA_POLLER_H
#define NEBLINA_POLLER_H

#include <stdbool.h>
#include <stddef.h>

#include "../socket.h"

typedef struct Poller Poller;

typedef enum { PT_NEW_CONNECTION, PT_NEW_DATA, PT_DISCONNECTED } PollerEventType;
typedef struct {
    PollerEventType type;
    SOCKET          fd;
} PollerEvent;

#define TIMEOUT 100

Poller* poller_create(SOCKET fd_listener);
void    poller_destroy(Poller* p);

size_t  poller_wait(Poller* p, PollerEvent* out_evt, size_t evt_sz, size_t timeout_ms);
bool    poller_add_connection(Poller* p, SOCKET fd);
bool    poller_remove_connection(Poller* p, SOCKET fd);

#endif //NEBLINA_POLLER_H
