#ifndef NEBLINA_SERVER_SESSION_H
#define NEBLINA_SERVER_SESSION_H

typedef struct Session Session;
typedef struct Connection Connection;

typedef int(*SessionOnRecv)(Session* session, Connection* connection);
typedef void(*SessionFree)(Session* session);

typedef struct SessionVTable {
    SessionOnRecv on_recv;
    SessionFree   session_free;
} SessionVTable;

typedef struct Session {
    SessionVTable const* vt;
} Session;

void session_init(Session* session, SessionVTable const* vt);
void session_free(Session* session);

int  session_on_recv(Session* session, Connection* connection);

#endif //NEBLINA_SERVER_SESSION_H
