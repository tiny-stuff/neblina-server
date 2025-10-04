#include "poller.h"

#include <sys/event.h>
#include <sys/time.h>

#include "common.h"

static int kqueue_fd = -1;
static int fs_socket = -1;

void poller_init(int fd_listener)
{
    kqueue_fd = kqueue();
    if (kqueue_fd < -1)
        FATAL("Could not initialize epoll: %s", strerror(errno));

    struct kevent event;
    EV_SET(&event, fd_listener, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
        FATAL("Could not initialize socket fd in epoll: %s", strerror(errno));

    fs_socket = fd_listener;
}

bool poller_add_connection(int fd)
{
    struct kevent event;
    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
        THROW("Could not add socket fd to kqueue: %s", strerror(errno));

    return true;
}

bool poller_remove_connection(SOCKET fd)
{
    struct kevent del_event;
    EV_SET(&del_event, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    if (kevent(kqueue_fd, &del_event, 1, NULL, 0, NULL))
        THROW("Could not add socket fd to kqueue: %s", strerror(errno));
    return true;
}


size_t poller_wait(PollerEvent* out_evt, size_t evt_sz)
{
    struct kevent events[evt_sz];
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = 100 * 1000000 }; // 100ms

    int n_events = kevent(kqueue_fd, NULL, 0, events, evt_sz, &timeout);
    if (n_events < 0 && errno != EINTR)
        THROW("kevent wait failed: %s", strerror(errno));

    for (int i = 0; i < n_events; ++i) {
        int fd = events[i].ident;
        if (fd == fs_socket)
            out_evt[i] = (PollerEvent){ .type = PT_NEW_CONNECTION, .fd = fs_socket };
        else if (events[i].flags & EV_EOF)
            out_evt[i] = (PollerEvent){ .type = PT_DISCONNECTED, .fd = fd };
        else
            out_evt[i] = (PollerEvent){ .type = PT_NEW_DATA, .fd = fd };
    }

    return n_events;
}
