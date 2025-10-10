#ifndef NEBLINA_SERVER_SESSION_H
#define NEBLINA_SERVER_SESSION_H

#include "socket.h"

typedef struct Session Session;
typedef struct CommunicationBuffer CommunicationBuffer;

typedef int(*SessionOnRecv)(Session* session, CommunicationBuffer* connection);
typedef void(*SessionFinalize)(Session* session);

typedef struct SessionVTable {
    SessionOnRecv   on_recv;
    SessionFinalize finalize;
} SessionVTable;

typedef struct Session {
    SessionVTable vt;
    SOCKET        fd;
    CommunicationBuffer*   connection;
} Session;

void session_init(Session* session, SOCKET fd, SessionOnRecv on_recv, SessionFinalize finalize);
void session_finalize(Session* session);

int  session_on_recv(Session* session);

#endif //NEBLINA_SERVER_SESSION_H
