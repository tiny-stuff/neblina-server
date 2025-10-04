#include "poller.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>

#include "common.h"

static SOCKET fs_socket = INVALID_SOCKET;
static WSAPOLLFD poll_fds[FD_SETSIZE];
static int poll_count = 0;

void poller_init(SOCKET fd_listener)
{
    fs_socket = fd_listener;

    poll_count = 0;
    poll_fds[poll_count].fd = fd_listener;
    poll_fds[poll_count].events = POLLRDNORM;  // readiness for reading (new connections)
    poll_count++;
}

bool poller_add_connection(SOCKET fd)
{
    if (poll_count >= FD_SETSIZE) {
        THROW("Too many sockets for WSAPoll()");
        return false;
    }

    poll_fds[poll_count].fd = fd;
    poll_fds[poll_count].events = POLLIN;
    poll_count++;

    return true;
}

bool poller_remove_connection(SOCKET fd)
{
    for (int i = 0; i < poll_count; i++) {
        if (poll_fds[i].fd == fd) {
            // shift left
            for (int j = i; j < poll_count - 1; j++) {
                poll_fds[j] = poll_fds[j + 1];
            }
            poll_count--;
            return true;
        }
    }
    THROW("Socket not found in poll list");
    return false;
}

size_t poller_wait(PollerEvent* out_evt, size_t evt_sz)
{
    int n_ready = WSAPoll(poll_fds, poll_count, 100);  // timeout 100 ms
    if (n_ready < 0) {
    	ERR("WSAPoll() error: %d", WSAGetLastError());
        return 0;
    }

    size_t out_count = 0;
    for (int i = 0; i < poll_count && out_count < evt_sz; i++) {
        short revents = poll_fds[i].revents;
        if (revents == 0)
            continue;

        if (poll_fds[i].fd == fs_socket) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_NEW_CONNECTION, .fd = fs_socket };
        } 
        if (poll_fds[i].fd != fs_socket && revents & (POLLERR | POLLHUP)) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_DISCONNECTED, .fd = poll_fds[i].fd };
        } 
        if (poll_fds[i].fd != fs_socket && revents & POLLIN) {
            out_evt[out_count++] = (PollerEvent) { .type = PT_NEW_DATA, .fd = poll_fds[i].fd };
        }
    }

    return out_count;
}
