#include "tcp_server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tcp_server_priv.h"
#include "util/logs.h"

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib")
#else
#  define _DARWIN_C_SOURCE
#  include <unistd.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  define INVALID_SOCKET (-1)
#  define SOCKET_ERROR (-1)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#endif

static const char* ERR_PRX = "TCP server error:";

static void close_socket(SOCKET fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

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
    free(server);
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
            close_socket(listener);
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

static void tcp_server_iterate(Server* server)
{
    // TODO
}

Server* tcp_server_create(int port, bool open_to_world, CreateSessionF create_session, size_t n_threads)
{
    static const ServerVTable vtable = {
        .recv = tcp_server_recv,
        .send = tcp_server_send,
        .free = tcp_server_free,
        .iterate = tcp_server_iterate,
    };

    TCPServer* tcp_server = calloc(1, sizeof(TCPServer));
    server_init(create_session, n_threads);
    tcp_server->server.vt = &vtable;
    tcp_server->port = port;
    tcp_server->open_to_world = open_to_world;
    tcp_server->server.fd = tcp_server_get_listener(port, open_to_world);
    return (Server *) tcp_server;
}