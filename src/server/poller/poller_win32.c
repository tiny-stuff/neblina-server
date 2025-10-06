#include "poller.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>

#include "util/error.h"
#include "util/logs.h"

typedef struct Poller {
    SOCKET fs_socket;
    WSAPOLLFD poll_fds[FD_SETSIZE];
    int poll_count;
} Poller;


Poller* poller_create(SOCKET fd_listener)
{
	Poller* p = (Poller*)malloc(sizeof(Poller));
    p->fs_socket = fd_listener;

    p->poll_count = 0;
    p->poll_fds[p->poll_count].fd = fd_listener;
    p->poll_fds[p->poll_count].events = POLLRDNORM;  // readiness for reading (new connections)
    p->poll_count++;

    return p;
}

void poller_destroy(Poller* p)
{
    free(p);
}

bool poller_add_connection(Poller* p, SOCKET fd)
{
    if (p->poll_count >= FD_SETSIZE) {
        FATAL_NON_RECOVERABLE("Too many sockets for WSAPoll()");
        return false;
    }

    p->poll_fds[p->poll_count].fd = fd;
    p->poll_fds[p->poll_count].events = POLLIN;
    p->poll_count++;

    return true;
}

bool poller_remove_connection(Poller* p, SOCKET fd)
{
    for (int i = 0; i < p->poll_count; i++) {
        if (p->poll_fds[i].fd == fd) {
            // shift left
            for (int j = i; j < p->poll_count - 1; j++) {
                p->poll_fds[j] = p->poll_fds[j + 1];
            }
            p->poll_count--;
            return true;
        }
    }
    FATAL_NON_RECOVERABLE("Socket not found in poll list");
}

size_t poller_wait(Poller* p, PollerEvent* out_evt, size_t evt_sz, size_t timeout_ms)
{
    int n_ready = WSAPoll(p->poll_fds, p->poll_count, 100);  // timeout 100 ms
    if (n_ready < 0) {
    	ERR("WSAPoll() error: %d", WSAGetLastError());
        return 0;
    }

    size_t out_count = 0;
    for (int i = 0; i < p->poll_count && out_count < evt_sz; i++) {
        short revents = p->poll_fds[i].revents;
        if (revents == 0)
            continue;

        if (p->poll_fds[i].fd == p->fs_socket) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_NEW_CONNECTION, .fd = p->fs_socket };
        } 
        if (p->poll_fds[i].fd != p->fs_socket && revents & (POLLERR | POLLHUP)) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_DISCONNECTED, .fd = p->poll_fds[i].fd };
        } 
        if (p->poll_fds[i].fd != p->fs_socket && revents & POLLIN) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_NEW_DATA, .fd = p->poll_fds[i].fd };
        }
    }

    return out_count;
}
