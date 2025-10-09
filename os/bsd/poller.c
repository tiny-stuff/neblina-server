#include "poller.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/time.h>

#include "util/logs.h"
#include "util/error.h"
#include "util/alloc.h"

typedef struct Poller {
    int kqueue_fd;
    int fs_socket;
} Poller;

Poller* poller_create(SOCKET fd_listener)
{
    Poller* p = CALLOC(1, sizeof(Poller));

    p->kqueue_fd = kqueue();
    if (p->kqueue_fd < -1)
        FATAL_NON_RECOVERABLE("Could not initialize epoll: %s", strerror(errno));

    struct kevent event;
    EV_SET(&event, fd_listener, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
        FATAL_NON_RECOVERABLE("Could not initialize socket fd in epoll: %s", strerror(errno));

    p->fs_socket = fd_listener;

    return p;
}

void poller_destroy(Poller* p)
{
    close(p->kqueue_fd);
    free(p);
}

bool poller_add_connection(Poller* p, int fd)
{
    struct kevent event;
    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
        FATAL_NON_RECOVERABLE("Could not add socket fd to kqueue: %s", strerror(errno));

    return true;
}

bool poller_remove_connection(Poller* p, SOCKET fd)
{
    struct kevent del_event;
    EV_SET(&del_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(p->kqueue_fd, &del_event, 1, NULL, 0, NULL))
        FATAL_NON_RECOVERABLE("Could not add socket fd to kqueue: %s", strerror(errno));
    return true;
}


size_t poller_wait(Poller* p, PollerEvent* out_evt, size_t evt_sz, size_t timeout_ms)
{
    struct kevent events[evt_sz];
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = timeout_ms * 1000000 };

    int n_events = kevent(p->kqueue_fd, NULL, 0, events, (int) evt_sz, &timeout);
    if (n_events < 0 && errno != EINTR)
        FATAL_NON_RECOVERABLE("kevent wait failed: %s", strerror(errno));

    for (int i = 0; i < n_events; ++i) {
        int fd = (int) events[i].ident;
        if (fd == p->fs_socket)
            out_evt[i] = (PollerEvent){ .type = PT_NEW_CONNECTION, .fd = p->fs_socket };
        else if (events[i].flags & EV_EOF)
            out_evt[i] = (PollerEvent){ .type = PT_DISCONNECTED, .fd = fd };
        else
            out_evt[i] = (PollerEvent){ .type = PT_NEW_DATA, .fd = fd };
    }

    return n_events;
}
