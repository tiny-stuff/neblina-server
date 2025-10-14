#include "doctest.h"

#include <string.h>
#include <stdlib.h>

extern "C" {
#include "server/commbuf.h"
#include "service/session.h"
#include "server/tcp/tcp_server.h"
#include "client/tcpclient.h"

extern bool logs_enabled;
}

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

    ParrotSession* psession = (ParrotSession *) calloc(1, sizeof(ParrotSession));
    session_init(&psession->session, fd, parrot_on_recv, NULL);
    return (Session *) psession;
}

TEST_CASE("TCP (single threaded)")
{
    logs_enabled = false;

    TCPServer* server = tcp_server_create(23456, false, parrot_session_create, 8);
    CHECK(server);

    TCPClient* client = tcpclient_create("127.0.0.1", 23456);
    CHECK(client);

    SUBCASE("Send data to server and get echo")
    {
        CHECK(tcpclient_send_text(client, "hello\r\n") == 7);
        server_iterate((Server *) server, 50);  // 1st iteration, accept connection
        server_iterate((Server *) server, 50);  // 2nd iteration, process incoming data

        char resp[6] = {0};
        ssize_t r = tcpclient_recv_spinlock(client, (uint8_t *) resp, 5, 5000);
        CHECK(memcmp(resp, "hello", r) == 0);
    }

    tcpclient_destroy(client);
    tcp_server_destroy(server);

    logs_enabled = true;
}
