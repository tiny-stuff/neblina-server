#ifndef NEBLINA_SERVER_CONNECTION_H
#define NEBLINA_SERVER_CONNECTION_H

#include <stdint.h>
#include <stddef.h>
#include "socket.h"

typedef struct CommunicationBuffer CommunicationBuffer;

CommunicationBuffer* commbuf_create();
void                 commbuf_destroy(CommunicationBuffer* c);

void                 commbuf_add_to_recv_buffer(CommunicationBuffer* c, uint8_t const* data, size_t data_sz);
void                 commbuf_add_to_send_buffer(CommunicationBuffer* c, uint8_t const* data, size_t data_sz);
void                 commbuf_add_text_to_send_buffer(CommunicationBuffer* c, const char* text);

void                 commbuf_clear_send_buffer(CommunicationBuffer* c);

uint8_t const*       commbuf_send_buffer(CommunicationBuffer const* c, size_t* data_sz);

size_t               commbuf_extract_from_recv_buffer(CommunicationBuffer* c, uint8_t** data);
size_t               commbuf_extract_line_from_recv_buffer(CommunicationBuffer* c, char** data, const char* separator);

#endif //NEBLINA_SERVER_CONNECTION_H
