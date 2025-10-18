#ifndef CLIENT_H_
#define CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#include "socket.h"
#include "util/future.h"

typedef struct Client Client;

ssize_t client_send(Client* t, uint8_t* data, size_t sz);
ssize_t client_send_text(Client* t, const char* data);

ssize_t client_recv_nonblock(Client* t, uint8_t** data);
ssize_t client_recv_spinlock(Client* t, uint8_t* data, size_t sz, size_t timeout_ms);

#endif
