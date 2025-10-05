#include "tcp_server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_server_priv.h"
#include "util/logs.h"

static const char* ERR_PRX = "TCP server error:";

static int tcp_server_recv(SOCKET fd, uint8_t** data)
{
    return 0;
}

static int tcp_server_send(SOCKET fd, uint8_t const* data, size_t data_sz)
{
    return 0;
}

static void tcp_server_free(Server* server)
{
    TCPServer* tserver = (TCPServer *) server;
    free(tserver);
}

static SOCKET tcp_server_get_listener(int port, bool open_to_world)
{
#define FATAL(...) { ERR(__VA_ARGS__); return -1; }

    SOCKET listener = -1;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        FATAL("WSAStartup() error!");
    DBG("WSAStartup() succeeded");
#endif

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
#ifdef _WIN32
        char yes = '1';
#else
        int yes = 1;
#endif
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERROR)
            FATAL("%s setsocket error: %s", ERR_PRX, strerror(errno));

        // bind to port
        if (bind(listener, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
            server_close_socket(listener);
            continue;  // not possible, try next
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
        FATAL("%s failed to bind: %s", ERR_PRX, strerror(errno));

    if (listen(listener, 10) == SOCKET_ERROR)
        FATAL("%s listen error: %s", ERR_PRX, strerror(errno));

    LOG("listening in port %d", port);
    DBG("with fd %d", listener);

    return listener;
}

static SOCKET tcp_accept_new_connection(Server* server)
{
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
    char hoststr[NI_MAXHOST] = "Unknown";
    char portstr[NI_MAXSERV] = "0";
    getnameinfo((struct sockaddr const*)(&remoteaddr), addrlen, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
    DBG("New connection from %s:%s as fd %d", hoststr, portstr, client_fd);

    return client_fd;
}

Server* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session, size_t n_threads)
{
    static const ServerVTable vtable = {
        .free = tcp_server_free,
        .recv = tcp_server_recv,
        .send = tcp_server_send,
        .accept_new_connection = tcp_accept_new_connection,
    };

    int fd = tcp_server_get_listener(port, open_to_world);

    TCPServer* tcp_server = calloc(1, sizeof(TCPServer));
    server_init(&tcp_server->server, fd, create_session, NULL, n_threads);
    tcp_server->server.vt = &vtable;
    tcp_server->port = port;
    tcp_server->open_to_world = open_to_world;
    return (Server *) tcp_server;
}