#include "tcpclient.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "os.h"
#include "tcpclient_priv.h"
#include "util/alloc.h"
#include "util/logs.h"

#define BUF_SZ (16 * 1024)

static ssize_t recv_(SOCKET fd, uint8_t* data, size_t sz)
{
    return recv(fd, data, sz, 0);
}

static ssize_t send_(SOCKET fd, uint8_t const* data, size_t sz)
{
    return send(fd, data, sz, 0);
}

static void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static SOCKET open_connection_(const char* host, int port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    struct addrinfo* servinfo;
    char sport[15]; snprintf(sport, sizeof sport, "%d", port);
    if ((rv = getaddrinfo(host, sport, &hints, &servinfo)) != 0) {
        ERR("getaddrinfo: %s", gai_strerror(rv));
        return INVALID_SOCKET;
    }

    SOCKET sockfd = INVALID_SOCKET;
    struct addrinfo* p;
    char address[INET6_ADDRSTRLEN];
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, sizeof address);
        DBG("client: attempting connection to %s:%s", address, sport);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    ERR("connect() to %s:%s failed: %s", address, sport, strerror(errno));
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        ERR("Failed to connect.");
        return INVALID_SOCKET;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), address, sizeof address);
    DBG("client: connected to %s:%s", address, sport);

#ifdef _WIN32
    unsigned long mode = 1;
    if (ioctlsocket(sockfd, FIONBIO, &mode) != 0)
        ERR("client: error marking socket as non-blocking");
#else
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        ERR("fcntl: %s", strerror(errno));
        return false;
    }
    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) < 0)
        ERR("client: error marking socket as non-blocking");
#endif

    freeaddrinfo(servinfo);

    return sockfd;
}

int tcpclient_initialize(TCPClient* t, const char* host, int port)
{
    memset(t, 0, sizeof(*t));
    t->vt.recv = recv_;
    t->vt.send = send_;
    t->fd = open_connection_(host, port);
    return (t->fd != INVALID_SOCKET) ? 0 : -1;
}

void tcpclient_finalize(TCPClient* t)
{
    if (t->fd != INVALID_SOCKET)
        close(t->fd);
}

TCPClient* tcpclient_create(const char* host, int port)
{
    TCPClient* tcpclient = malloc(sizeof(TCPClient));
    if (tcpclient == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    int r = tcpclient_initialize(tcpclient, host, port);
    if (r < 0) {
        tcpclient_destroy(tcpclient);
        return NULL;
    }
    return tcpclient;
}

void tcpclient_destroy(TCPClient* tcpclient)
{
    tcpclient_finalize(tcpclient);
    free(tcpclient);
}

ssize_t tcpclient_send(TCPClient* t, uint8_t* data, size_t sz)
{
    return t->vt.send(t->fd, data, sz);
}

ssize_t tcpclient_send_text(TCPClient* t, const char* data)
{
    return t->vt.send(t->fd, (uint8_t const *) data, strlen(data));
}

ssize_t tcpclient_recv(TCPClient* t, uint8_t** data)
{
    *data = MALLOC(BUF_SZ);
    ssize_t r = t->vt.recv(t->fd, *data, BUF_SZ);
    if (r <= 0) {
        free(data);
    }

    return r;
}

ssize_t tcpclient_recv_spinlock(TCPClient* t, uint8_t* data, size_t sz)
{
    size_t total_sz = 0;
    while (total_sz < sz) {
        ssize_t r = t->vt.recv(t->fd, data, sz - total_sz);
        if (r <= 0 && errno != EAGAIN) {
            ERR("client: recv: %s", strerror(errno));
            return r;
        }

        if (r > 0) {
            total_sz += r;
            data += r;
        }

        if (total_sz < sz)
            os_sleep_ms(1);
    }

    return 0;
}
