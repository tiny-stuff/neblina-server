#include "poller.h"

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

#include "util/error.h"

typedef struct Poller {
    int epoll_fd;
    int fs_socket;
} Poller;

Poller* poller_init(int fd_listener)
{
    Poller* p = calloc(1, sizeof(Poller));

    p->epoll_fd = epoll_create1(0); // 0 for default flags
    if (p->epoll_fd < -1)
        FATAL_NON_RECOVERABLE("Could not initialize epoll: %s", strerror(errno));

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd_listener;
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, fd_listener, &event) < 0)
        FATAL_NON_RECOVERABLE("Could not initialize socket fd in epoll: %s", strerror(errno));

    p->fs_socket = fd_listener;

    return p;
}

void poller_destroy(Poller* p)
{
    free(p);
}

bool poller_add_connection(Poller* p, int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    event.data.fd = fd;
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
        FATAL_NON_RECOVERABLE("Could not initialize socket fd in epoll: %s", strerror(errno));

    return true;
}

bool poller_remove_connection(Poller* p, SOCKET fd)
{
    if (epoll_ctl(p->epoll_fd, EPOLL_CTL_DEL, fd, NULL) < 0)
        FATAL_NON_RECOVERABLE("Error deleting socket fd in epoll: %s", strerror(errno));
    return true;
}

size_t poller_wait(Poller* p, PollerEvent* out_evt, size_t evt_sz)
{
    struct epoll_event events[evt_sz];
    int n_events = epoll_wait(p->epoll_fd, events, (int) evt_sz, 100);

    if (n_events < 0) {
        if (errno == EINTR)
            return 0;
        else
            FATAL_NON_RECOVERABLE("epoll error: %s", strerror(errno));
    }

    for (int i = 0; i < n_events; ++i) {
        if (events[i].data.fd == p->fs_socket) {
            out_evt[i] = (PollerEvent) { .type = PT_NEW_CONNECTION, .fd = p->fs_socket };
        } else {
            if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                out_evt[i] = (PollerEvent) { .type = PT_DISCONNECTED, .fd = events[i].data.fd };
            } else if (events[i].events & (EPOLLIN | EPOLLET)) {
                out_evt[i] = (PollerEvent) { .type = PT_NEW_DATA, .fd = events[i].data.fd };
            }
        }
    }

    return n_events;
}