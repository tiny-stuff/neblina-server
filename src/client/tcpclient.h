#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <stddef.h>
#include <stdint.h>

typedef struct TCPClient TCPClient;

TCPClient* tcpclient_create(const char* host, int port);
void       tcpclient_destroy(TCPClient* t);

int        tcpclient_send(TCPClient* t, uint8_t* data, size_t sz);
int        tcpclient_send_text(TCPClient* t, const char* data);

int        tcpclient_recv(TCPClient* t, uint8_t** data);
int        tcpclient_recv_spinlock(TCPClient* t, uint8_t* data, size_t sz);

#endif
