#ifndef NEBLINA_SERVER_CONNECTION_H
#define NEBLINA_SERVER_CONNECTION_H

#include <stdint.h>
#include <stddef.h>

typedef struct Connection Connection;

Connection*    connection_create(int fd);
void           connection_destroy(Connection* c);

void           connection_add_to_recv_buffer(Connection* c, uint8_t const* data, size_t data_sz);
void           connection_add_to_send_buffer(Connection* c, uint8_t const* data, size_t data_sz);

void           connection_clear_buffers(Connection* c);

int            connection_fd(Connection const* c);
uint8_t const* connection_recv_buffer(Connection const* c, size_t* data_sz);
uint8_t const* connection_send_buffer(Connection const* c, size_t* data_sz);

#endif //NEBLINA_SERVER_CONNECTION_H
