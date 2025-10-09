#include "session.h"

#include <stdlib.h>

#include "../server/connection.h"

void session_init(Session* session, SOCKET fd, SessionOnRecv on_recv, SessionFinalize finalize)
{
    session->connection = connection_create(fd);
    session->vt.on_recv = on_recv;
    session->vt.finalize = finalize;
}

int session_on_recv(Session* session)
{
    return session->vt.on_recv(session, session->connection);
}

void session_finalize(Session* session)
{
    connection_destroy(session->connection);
    if (session->vt.finalize)
        session->vt.finalize(session);
}