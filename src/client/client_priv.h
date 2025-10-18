#ifndef CLIENT_PRIV_H_
#define CLIENT_PRIV_H_

#include "socket.h"

typedef struct ClientVTable {
    ssize_t (*recv)(SOCKET fd, uint8_t* data, size_t sz);
    ssize_t (*send)(SOCKET fd, uint8_t const* data, size_t sz);
} ClientVTable;

typedef struct Client {
    ClientVTable vt;
    SOCKET fd;
} Client;

int  client_initialize(Client* client, SOCKET fd);
void client_finalize(Client* client);

#endif
