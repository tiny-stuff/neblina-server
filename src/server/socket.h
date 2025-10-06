#ifndef SOCKET_H_
#define SOCKET_H_

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "Ws2_32.lib")
#else
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

#endif