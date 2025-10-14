#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <string.h>

extern "C" {
#include "server/commbuf.h"
const char* service = "tests";
}

TEST_CASE("Communication Buffer")
{
    size_t sz;
    uint8_t* data;
    CommunicationBuffer* conn = commbuf_create();

    SUBCASE("Multiple sends")
    {
        commbuf_add_to_send_buffer(conn, (uint8_t const*) "Hello", 5);
        commbuf_add_to_send_buffer(conn, (uint8_t const*) "World", 5);

        uint8_t const* data = commbuf_send_buffer(conn, &sz);
        CHECK(sz == 10);
        CHECK(memcmp(data, "HelloWorld", sz) == 0);
    }

    SUBCASE("Clear buffer")
    {
        commbuf_clear_send_buffer(conn);
        commbuf_send_buffer(conn, &sz);
        CHECK(sz == 0);
    }

    SUBCASE("Receive buffer")
    {
        commbuf_add_to_recv_buffer(conn, (uint8_t const*) "Hello", 5);
        sz = commbuf_extract_from_recv_buffer(conn, &data);
        CHECK(sz == 5);
        CHECK(memcmp(data, (uint8_t const*) "Hello", sz) == 0);
        free(data);
    }

    SUBCASE("Line breaks")
    {
        char* data3;
        commbuf_add_to_recv_buffer(conn, (uint8_t const*) "Hello\nWorld\ntest", 16);
        sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
        CHECK(sz == 6);
        CHECK(memcmp(data3, (uint8_t const*) "Hello\n", sz) == 0);
        free(data3);

        sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
        CHECK(sz == 6);
        CHECK(memcmp(data3, (uint8_t const*) "World\n", sz) == 0);
        free(data3);

        sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
        CHECK(sz == 0);
    }

    commbuf_destroy(conn);
}