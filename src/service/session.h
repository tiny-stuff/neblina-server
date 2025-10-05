#ifndef NEBLINA_SERVER_SESSION_H
#define NEBLINA_SERVER_SESSION_H

typedef struct Session Session;
typedef struct Connection Connection;

typedef int(*SessionOnRecv)(Session* session, Connection* connection);
typedef void(*SessionFinalize)(Session* session);

typedef struct SessionVTable {
    SessionOnRecv   on_recv;
    SessionFinalize finalize;
} SessionVTable;

typedef struct Session {
    SessionVTable vt;
} Session;

void session_init(Session* session, SessionOnRecv on_recv, SessionFinalize finalize);
void session_finalize(Session* session);

int  session_on_recv(Session* session, Connection* connection);

#endif //NEBLINA_SERVER_SESSION_H
