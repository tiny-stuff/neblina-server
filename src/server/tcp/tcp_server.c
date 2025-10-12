#include "tcp_server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_server_priv.h"
#include "util/logs.h"
#include "socket.h"
#include "util/alloc.h"

static const char* ERR_PRX = "TCP server error:";

static int tcp_server_recv(SOCKET fd, uint8_t** data)
{
    *data = MALLOC(RECV_BUF_SZ);
    ssize_t r = recv(fd, (char *) *data, RECV_BUF_SZ, 0);
    if (r < 0)
        ERR("%s recv error: %s", ERR_PRX, strerror(errno));
    return (int) r;
}

static int tcp_server_send(SOCKET fd, uint8_t const* data, size_t data_sz)
{
    ssize_t r = send(fd, (const char *) data, (int) data_sz, 0);
    if (r < 0)
        ERR("%s send error: %s", ERR_PRX, strerror(errno));
    return (int) r;
}

static void tcp_server_free(Server* server)
{
    socket_finalize();

    TCPServer* tserver = (TCPServer *) server;
    free(tserver);
}

static SOCKET tcp_server_get_listener(int port, bool open_to_world)
{
#define FATAL(...) { ERR(__VA_ARGS__); if (listener != INVALID_SOCKET) close(listener); return INVALID_SOCKET; }

    SOCKET listener = INVALID_SOCKET;

    // find internet address to bind
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // use my IP

    const char* listen_to = open_to_world ? NULL: "localhost";

    int rv;
    struct addrinfo* servinfo;
    char sport[15]; snprintf(sport, sizeof sport, "%d", port);
    if ((rv = getaddrinfo(listen_to, sport, &hints, &servinfo)) != 0)
        FATAL("%s getaddrinfo error: %s", ERR_PRX, gai_strerror(rv));

    // loop through all the results and bind to the first we can
    struct addrinfo* p;
    for(p = servinfo; p != NULL; p = p->ai_next) {

        // open socket
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener == INVALID_SOCKET)
            FATAL("%s socket error: %s", ERR_PRX, strerror(errno));

        // set socket as reusable
        SOCKETOPT_YES
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR)
            FATAL("%s setsocket error: %s", ERR_PRX, strerror(errno));

        // bind to port
        if (bind(listener, p->ai_addr, (int) p->ai_addrlen) == SOCKET_ERROR) {
            server_close_socket(listener);
            continue;  // not possible, try next
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
        FATAL("%s failed to bind: %s", ERR_PRX, strerror(errno));

    if (listen(listener, SOMAXCONN) == SOCKET_ERROR)
        FATAL("%s listen error: %s", ERR_PRX, strerror(errno));

    LOG("listening in port %d", port);
    DBG("with fd %d", listener);

    return listener;
}

static SOCKET tcp_accept_new_connection(Server* server)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-fd-leak"

    // accept connection
    struct sockaddr_storage remoteaddr; // Client address
    memset(&remoteaddr, 0, sizeof remoteaddr);
    socklen_t addrlen = sizeof remoteaddr;

    SOCKET client_fd = accept(server->fd, (struct sockaddr *) &remoteaddr, &addrlen);
    if (client_fd == -1) {
        ERR("%s listen error: %s", ERR_PRX, strerror(errno));
        return INVALID_SOCKET;
    }

    // find connecter IP/port
    char hoststr[1024] = "Unknown";
    char portstr[24] = "0";
    if (getnameinfo((struct sockaddr const*)(&remoteaddr), addrlen, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
        DBG("New connection from %s:%s as fd %d", hoststr, portstr, client_fd);

    return client_fd;

#pragma GCC diagnostic pop
}

int tcp_server_initialize(TCPServer* tcp_server, int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads)
{
    static const ServerVTable vtable = {
            .recv = tcp_server_recv,
            .send = tcp_server_send,
            .accept_new_connection = tcp_accept_new_connection,
    };

    socket_init();

    SOCKET fd = tcp_server_get_listener(port, open_to_world);
    if (fd == INVALID_SOCKET)
        return -1;

    server_initialize(&tcp_server->server, fd, create_session_cb, NULL, n_threads);
    tcp_server->server.vt = &vtable;
    tcp_server->port = port;
    tcp_server->open_to_world = open_to_world;

    return 0;
}

TCPServer* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session_cb, size_t n_threads)
{
    TCPServer* tcp_server = CALLOC(1, sizeof(TCPServer));
    int r = tcp_server_initialize(tcp_server, port, open_to_world, create_session_cb, n_threads);
    if (r < 0) {
        tcp_server_free((Server *) tcp_server);
        return NULL;
    }
    return tcp_server;
}

void tcp_server_finalize(TCPServer* tcp_server)
{
    (void) tcp_server;
}

void tcp_server_destroy(TCPServer* tcp_server)
{
    tcp_server_finalize(tcp_server);
    server_finalize((Server *) tcp_server);
    free(tcp_server);
}
