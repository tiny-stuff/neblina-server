#include "session.h"

#include <stdlib.h>

void session_init(Session* session, SessionVTable const* vt)
{
    session->vt = vt;
}

int session_on_recv(Session* session, Connection* connection)
{
    return session->vt->on_recv(session, connection);
}

void session_free(Session* session)
{
    session->vt->session_free(session);
}