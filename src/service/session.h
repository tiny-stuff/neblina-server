#ifndef NEBLINA_SERVER_SESSION_H
#define NEBLINA_SERVER_SESSION_H

typedef struct Session Session;
typedef struct Connection Connection;

typedef int(*SessionOnRecv)(Session* session, Connection* connection);
typedef void(*SessionFree)(Session* session);

typedef struct Session {
    SessionOnRecv on_recv;
    SessionFree   session_free;
} Session;

void session_init(Session* session, SessionOnRecv on_recv, SessionFree session_free);

int session_on_recv(Session* session, Connection* connection);

#endif //NEBLINA_SERVER_SESSION_H
