#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <stddef.h>
#include <stdint.h>

#include "socket.h"
#include "util/future.h"

typedef struct TCPClient TCPClient;

TCPClient* tcpclient_create(const char* host, int port);
void       tcpclient_destroy(TCPClient* t);

ssize_t    tcpclient_send(TCPClient* t, uint8_t* data, size_t sz);
ssize_t    tcpclient_send_text(TCPClient* t, const char* data);

ssize_t    tcpclient_recv_nonblock(TCPClient* t, uint8_t** data);
ssize_t    tcpclient_recv_spinlock(TCPClient* t, uint8_t* data, size_t sz, size_t timeout_ms);

#endif
