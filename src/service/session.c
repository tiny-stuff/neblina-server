#include "session.h"

#include <stdlib.h>

void session_init(Session* session, SessionOnRecv on_recv, SessionFree session_free)
{
    session->on_recv = on_recv;
    session->session_free = session_free;
}

int session_on_recv(Session* session, Connection* connection)
{
    return session->on_recv(session, connection);
}