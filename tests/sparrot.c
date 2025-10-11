#include "server/ssl/ssl_server.h"
#include "service/session.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util/logs.h"
#include "os/os.h"

const char* service = "parrot";

typedef struct ParrotSession {
    Session session;
} ParrotSession;

static int parrot_on_recv(Session* session, CommunicationBuffer* c)
{
    (void) session;

    char* line;
    while ((commbuf_extract_line_from_recv_buffer(c, &line, "\r\n"))) {
        commbuf_add_text_to_send_buffer(c, line);
        free(line);
    }

    return 0;
}

static Session* parrot_session_create(SOCKET fd, void* data)
{
    (void) data;

    ParrotSession* psession = calloc(1, sizeof(ParrotSession));
    session_init(&psession->session, fd, parrot_on_recv, NULL);
    return (Session *) psession;
}

int main(int argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        logs_verbose = true;
    os_handle_ctrl_c();

    Server* server = ssl_server_create(23457, false, parrot_session_create, 0);
    if (!server) {
        perror("ssl_create_socket");
        return EXIT_FAILURE;
    }

    server_run(server);
    server_destroy(server);
}
