#include "server/tcp/tcp_server.h"
#include "service/session.h"

#include <stdlib.h>
#include <string.h>

#include "util/logs.h"
#include "os/os.h"

const char* service = "parrot";

typedef struct ParrotSession {
    Session session;
} ParrotSession;

static int parrot_on_recv(Session* session, Connection* c)
{
    (void) session;

    char* line;
    while ((connection_extract_line_from_recv_buffer(c, &line, "\r\n"))) {
        connection_add_text_to_send_buffer(c, line);
        free(line);
    }

    return 0;
}

static Session* parrot_session_create(void* data)
{
    (void) data;

    ParrotSession* psession = calloc(1, sizeof(ParrotSession));
    session_init(&psession->session, parrot_on_recv, NULL);
    return (Session *) psession;
}

int main()
{
    logs_verbose = true;
    os_handle_ctrl_c();

    Server* server = tcp_server_create(23456, false, parrot_session_create, SINGLE_THREADED);
    server_run(server);
    server_destroy(server);
}
