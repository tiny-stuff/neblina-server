#include "socket.h"

#include "util/error.h"

void socket_init()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        FATAL_NON_RECOVERABLE("WSAStartup() error!");
    DBG("WSAStartup() succeeded");
}

void socket_finalize()
{
    WSACleanup();
}

void close(SOCKET fd)
{
    closesocket(fd);
}
