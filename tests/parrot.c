#include "server/server.h"
#include "server/cpool/cpool.h"
#include "service/session.h"

#include <stdlib.h>
#include <string.h>

const char service[] = "parrot";

/*
    size_t sz;
    char* line;
    while ((sz = connection_extract_line_from_recv_buffer(c, &line, "\r\n"))) {
        connection_add_to_send_buffer(c, (uint8_t *) line, strlen(line));
        free(line);
    }
    */

typedef struct ParrotSession {
    Session*    session;
    Connection* connection;
} ParrotSession;

static Session* parrot_session_create(Connection* c, void* data)
{
    ParrotSession* session = calloc(1, sizeof(ParrotSession));
    session->session = session_create(parrot_on_recv, free);
    session->connection = c;
    return (Session *) session;
}

int main()
{
    Server* server = tcp_server_create(parrot_session_create, SINGLE_THREADED);
    server_run(server);
}
