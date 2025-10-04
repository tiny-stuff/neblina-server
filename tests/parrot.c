#include "server/tcp/tcp_server.h"
#include "service/session.h"

#include <stdlib.h>
#include <string.h>

const char* service = "parrot";

typedef struct ParrotSession {
    Session session;
} ParrotSession;

static int parrot_on_recv(Session* session, Connection* c)
{
    (void) session;

    char* line;
    while ((connection_extract_line_from_recv_buffer(c, &line, "\r\n"))) {
        connection_add_to_send_buffer(c, (uint8_t *) line, strlen(line));
        free(line);
    }

    return 0;
}

static void parrot_free(Session* session)
{
    free(session);
}

static Session* parrot_session_create(void* data)
{
    (void) data;

    static const SessionVTable vt = {
        .on_recv = parrot_on_recv,
        .session_free = parrot_free,
    };

    ParrotSession* psession = calloc(1, sizeof(ParrotSession));
    session_init(&psession->session, &vt);
    return (Session *) psession;
}

int main()
{
    Server* server = tcp_server_create(23456, false, parrot_session_create, SINGLE_THREADED);
    server_run(server);
}
