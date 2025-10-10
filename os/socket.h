#ifndef SOCKET_H_
#define SOCKET_H_

#ifdef _WIN32

#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib")
#  define SOCKETOPT_YES char yes = '1';

typedef long ssize_t;

void socket_init();
void socket_finalize();

void close(SOCKET fd);

#else

#  include <unistd.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>

#  define INVALID_SOCKET (-1)
#  define SOCKET_ERROR (-1)
#  define SOCKETOPT_YES int yes = 1;

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define socket_init() {}
#define socket_finalize() {}

#endif

#include <fcntl.h>

#endif