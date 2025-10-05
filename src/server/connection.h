#ifndef NEBLINA_SERVER_CONNECTION_H
#define NEBLINA_SERVER_CONNECTION_H

#include <stdint.h>
#include <stddef.h>
#include "socket.h"

typedef struct Connection Connection;
typedef struct Session Session;

Connection*    connection_create(int fd, Session* session);
void           connection_destroy(Connection* c);

void           connection_add_to_recv_buffer(Connection* c, uint8_t const* data, size_t data_sz);
void           connection_add_to_send_buffer(Connection* c, uint8_t const* data, size_t data_sz);
void           connection_add_text_to_send_buffer(Connection* c, const char* text);

void           connection_clear_send_buffer(Connection* c);

SOCKET         connection_socket_fd(Connection const* c);
Session*       connection_session(Connection* c);
uint8_t const* connection_send_buffer(Connection const* c, size_t* data_sz);

size_t         connection_extract_from_recv_buffer(Connection* c, uint8_t** data);
size_t         connection_extract_line_from_recv_buffer(Connection* c, char** data, const char* separator);

#endif //NEBLINA_SERVER_CONNECTION_H
