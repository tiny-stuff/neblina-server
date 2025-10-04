#include "session.h"

#include <stdlib.h>

typedef struct Session {
    SessionOnRecv on_recv;
} Session;

Session* session_create(SessionOnRecv on_recv)
{
    Session* session = calloc(1, sizeof(Session));
    session->on_recv = on_recv;
    return session;
}

void session_destroy(Session* session)
{
    free(session);
}

int session_on_recv(Session* session, Connection* connection)
{
    return session->on_recv(session, connection);
}