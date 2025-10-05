#include "session.h"

#include <stdlib.h>

void session_init(Session* session, SessionOnRecv on_recv, SessionFinalize finalize)
{
    session->vt.on_recv = on_recv;
    session->vt.finalize = finalize;
}

int session_on_recv(Session* session, Connection* connection)
{
    return session->vt.on_recv(session, connection);
}

void session_finalize(Session* session)
{
    if (session->vt.finalize)
        session->vt.finalize(session);
}