#include "session.h"

#include <stdlib.h>

#include "../server/commbuf.h"

void session_init(Session* session, SOCKET fd, SessionOnRecv on_recv, SessionFinalize finalize)
{
    session->vt.on_recv = on_recv;
    session->vt.finalize = finalize;
    session->connection = commbuf_create();
    session->fd = fd;
}

int session_on_recv(Session* session)
{
    return session->vt.on_recv(session, session->connection);
}

void session_finalize(Session* session)
{
    commbuf_destroy(session->connection);
    if (session->vt.finalize)
        session->vt.finalize(session);
}