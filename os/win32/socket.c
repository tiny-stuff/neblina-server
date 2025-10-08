#include "socket.h"

void init_socket()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        FATAL("WSAStartup() error!");
    DBG("WSAStartup() succeeded");
}

void finalize_socket()
{
    WSACleanup()
}

void close(SOCKET fd)
{
    closesocket(fd);
}
