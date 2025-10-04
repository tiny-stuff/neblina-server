#ifndef NEBLINA_SERVER_SESSION_H
#define NEBLINA_SERVER_SESSION_H

typedef struct Session Session;
typedef struct Connection Connection;

typedef int(*SessionOnRecv)(Session* session, Connection* connection);

Session* session_create(SessionOnRecv on_recv);
void     session_destroy(Session* session);

int session_on_recv(Session* session, Connection* connection);

#endif //NEBLINA_SERVER_SESSION_H
