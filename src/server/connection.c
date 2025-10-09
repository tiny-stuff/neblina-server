#include "connection.h"

#include <stdlib.h>
#include <string.h>

#include "util/error.h"
#include "util/alloc.h"

typedef struct Connection {
    SOCKET   fd;
    uint8_t* recv_buf;
    size_t   recv_buf_sz;
    uint8_t* send_buf;
    size_t   send_buf_sz;
} Connection;

Connection* connection_create(SOCKET fd)
{
    Connection* c = CALLOC(1, sizeof(Connection));
    c->fd = fd;
    return c;
}

void connection_destroy(Connection* c)
{
    free(c->recv_buf);
    free(c->send_buf);
    free(c);
}

void connection_add_to_recv_buffer(Connection* c, uint8_t const* data, size_t data_sz)
{
    c->recv_buf = REALLOC(c->recv_buf, c->recv_buf_sz + data_sz);
    if (!c->recv_buf)
        FATAL_NON_RECOVERABLE("Allocation error");
    memcpy(&c->recv_buf[c->recv_buf_sz], data, data_sz);
    c->recv_buf_sz += data_sz;
}

void connection_add_to_send_buffer(Connection* c, uint8_t const* data, size_t data_sz)
{
    c->send_buf = REALLOC(c->send_buf, c->send_buf_sz + data_sz);
    if (!c->send_buf)
    FATAL_NON_RECOVERABLE("Allocation error");
    memcpy(&c->send_buf[c->send_buf_sz], data, data_sz);
    c->send_buf_sz += data_sz;
}

void connection_add_text_to_send_buffer(Connection* c, const char* text)
{
    connection_add_to_send_buffer(c, (uint8_t const *) text, strlen(text));
}

void connection_clear_send_buffer(Connection* c)
{
    c->send_buf_sz = 0;
}

void connection_clear_recv_buffer(Connection* c)
{
    c->recv_buf_sz = 0;
}

SOCKET connection_socket_fd(Connection const* c)
{
    return c->fd;
}

uint8_t const* connection_recv_buffer(Connection const* c, size_t* data_sz)
{
    *data_sz = c->recv_buf_sz;
    return c->recv_buf;
}

uint8_t const* connection_send_buffer(Connection const* c, size_t* data_sz)
{
    *data_sz = c->send_buf_sz;
    return c->send_buf;
}

size_t connection_extract_from_recv_buffer(Connection* c, uint8_t** data)
{
    size_t sz = c->recv_buf_sz;
    if (c->recv_buf_sz > 0) {
        *data = MALLOC(c->recv_buf_sz);
        memcpy(*data, c->recv_buf, c->recv_buf_sz);
        c->recv_buf_sz = 0;
    }
    return sz;
}

size_t connection_extract_line_from_recv_buffer(Connection* c, char** data, const char* separator)
{
    size_t sep_len = strlen(separator);
    int len_to_return = -1;

    // look for separator
    for (int i = 0; i < ((int) c->recv_buf_sz - (int) sep_len + 1); ++i) {
        if (memcmp(&c->recv_buf[i], separator, sep_len) == 0) {
            len_to_return = i;
            break;
        }
    }

    if (len_to_return == -1)  // enter not found
        return 0;

    // copy string and return
    len_to_return += (int) sep_len;  // include separator
    *data = CALLOC(1, len_to_return + 1);
    memcpy(*data, c->recv_buf, len_to_return);
    if ((int) c->recv_buf_sz > len_to_return)
        memmove(c->recv_buf, &c->recv_buf[len_to_return], c->recv_buf_sz - len_to_return);
    c->recv_buf_sz -= len_to_return;

    return len_to_return;
}
