#include "client.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "os.h"
#include "client_priv.h"
#include "util/alloc.h"
#include "util/logs.h"
#include "socket.h"

#define BUF_SZ (16 * 1024)

int client_initialize(Client* t, int fd)
{
    memset(t, 0, sizeof(*t));
    t->fd = fd;
    return (t->fd != INVALID_SOCKET) ? 0 : -1;
}

void client_finalize(Client* t)
{
    if (t->fd != INVALID_SOCKET)
        close_socket(t->fd);
}

void client_destroy(Client* client)
{
    client_finalize(client);
    free(client);
}

ssize_t client_send(Client* t, uint8_t* data, size_t sz)
{
    return t->vt.send(t, data, sz);
}

ssize_t client_send_text(Client* t, const char* data)
{
    return t->vt.send(t, (uint8_t const *) data, strlen(data));
}

ssize_t client_recv_nonblock(Client* t, uint8_t** data)
{
    *data = MALLOC(BUF_SZ);
    ssize_t r = t->vt.recv(t, *data, BUF_SZ);
    if (r <= 0) {
        if (r < 0 && errno == EAGAIN)
            r = 0;
        free(data);
    }

    return r;
}

ssize_t client_recv_spinlock(Client* t, uint8_t* data, size_t sz, size_t timeout_ms)
{
#ifdef _WIN32
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
#else
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    size_t pos = 0;
    while (pos < sz) {
        ssize_t r = t->vt.recv(t, &data[pos], sz - pos);
#ifdef _WIN32
		int err = WSAGetLastError();
        if (r < 0 && err != EWOULDBLOCK && err != WSAEWOULDBLOCK) {
            ERR("client: recv: %d", err);
#else
        if (r <= 0 && errno != EAGAIN) {
            ERR("client: recv: %s", strerror(errno));
#endif
            return r;
        }

        if (r > 0) {
            pos += r;
            data += r;
        }

        if (pos < sz)
            os_sleep_ms(1);

#ifdef _WIN32
        QueryPerformanceCounter(&end);
        double elapsed = (double)(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
#else
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (double) (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_nsec) / (double) 1e6;
#endif
        if (elapsed > (double) timeout_ms)
            return (ssize_t) pos;
    }

    return (ssize_t) pos;
}
