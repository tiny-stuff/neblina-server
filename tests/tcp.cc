#include "doctest.h"

#include <string.h>
#include <stdlib.h>

#include <thread>

extern "C" {
#include "os.h"
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

static std::atomic_bool server_running { true };
auto server_thread_function = []() {
    server_running = true;
    TCPServer* server = tcp_server_create(23456, false, parrot_session_create, 8);
    while (server_running)
        server_iterate((Server *) server, 50);
    tcp_server_destroy(server);
};

TEST_SUITE("TCP")
{
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

    TEST_CASE("TCP (server multithreaded, single client)")
    {
        logs_enabled = false;

        auto server_thread = std::thread(server_thread_function);
        os_sleep_ms(50);

        TCPClient* client = tcpclient_create("127.0.0.1", 23456);
        CHECK(client);

        SUBCASE("Send data to server and get echo")
        {
            CHECK(tcpclient_send_text(client, "hello\r\n") == 7);
            os_sleep_ms(50);

            char resp[6] = {0};
            ssize_t r = tcpclient_recv_spinlock(client, (uint8_t *) resp, 5, 5000);
            CHECK(memcmp(resp, "hello", r) == 0);
        }

        tcpclient_destroy(client);
        server_running = false;
        server_thread.join();

        logs_enabled = true;
    }

    TEST_CASE("TCP (server multithreaded, multiple single-threaded clients)")
    {
        logs_enabled = false;

        auto server_thread = std::thread(server_thread_function);
        os_sleep_ms(50);

#define N_CLIENTS 7
        TCPClient* clients[N_CLIENTS];
        for (size_t i = 0; i < N_CLIENTS; ++i) {
            clients[i] = tcpclient_create("127.0.0.1", 23456);
            CHECK(clients[i]);
        }

        for (size_t i = 0; i < N_CLIENTS; ++i)
            CHECK(tcpclient_send_text(clients[i], "hello\r\n") == 7);
        os_sleep_ms(50);

        for (size_t i = 0; i < N_CLIENTS; ++i) {
            char resp[6] = {0};
            ssize_t r = tcpclient_recv_spinlock(clients[i], (uint8_t *) resp, 5, 5000);
            CHECK(memcmp(resp, "hello", r) == 0);
        }

        for (size_t i = 0; i < N_CLIENTS; ++i)
            tcpclient_destroy(clients[i]);
        server_running = false;
        server_thread.join();

        logs_enabled = true;
    }
}