#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os/os.h"
#include "util/logs.h"
#include "server/connection.h"
#include "watchdog/watchdog.h"

const char* service = "tests";

//
// WATCHDOG TESTS
//

static void test_watchdog()
{
    WatchdogProgram programs[] = {
            { "error",   "tests/error",          NULL, 0 },
            { "infloop", "tests/infloop",        NULL, 0 },
            { "nonrec",  "tests/nonrecoverable", NULL, 0 },
    };
    watchdog_init(programs, sizeof programs / sizeof programs[0]);

    for (size_t i = 0; i < 5; ++i)
        watchdog_step();

    WatchdogProgramState error_state = watchdog_program_state(0);
    assert(error_state.attempts > 2);

    WatchdogProgramState infloop_state = watchdog_program_state(1);
    assert(infloop_state.status == WPS_RUNNING);
    assert(infloop_state.attempts == 1);
    assert(infloop_state.pid != PID_NOT_RUNNING);

    WatchdogProgramState nonrec_state = watchdog_program_state(2);
    assert(nonrec_state.status == WPS_GAVE_UP);
    assert(nonrec_state.attempts == 11);
    assert(nonrec_state.pid == PID_NOT_RUNNING);

    assert(os_process_still_running(infloop_state.pid, NULL));

    watchdog_finalize();

    os_sleep_ms(300);

    assert(!os_process_still_running(error_state.pid, NULL));
    assert(!os_process_still_running(infloop_state.pid, NULL));
    assert(!os_process_still_running(nonrec_state.pid, NULL));
}

//
// CONNECTION TESTS
//

static void test_connection()
{
    Connection* conn = connection_create(8);

    // send buffer

    connection_add_to_send_buffer(conn, (uint8_t const*) "Hello", 5);
    connection_add_to_send_buffer(conn, (uint8_t const*) "World", 5);

    assert(connection_socket_fd(conn) == 8);

    size_t sz;
    uint8_t const* data = connection_send_buffer(conn, &sz);
    assert(sz == 10);
    assert(memcmp(data, "HelloWorld", sz) == 0);

    connection_clear_send_buffer(conn);

    connection_send_buffer(conn, &sz);
    assert(sz == 0);

    connection_add_to_send_buffer(conn, (uint8_t const*) "Hello", 5);
    data = connection_send_buffer(conn, &sz);
    assert(sz == 5);
    assert(memcmp(data, "Hello", sz) == 0);

    // recv buffer

    connection_add_to_recv_buffer(conn, (uint8_t const*) "Hello", 5);
    uint8_t* data2;
    sz = connection_extract_from_recv_buffer(conn, &data2);
    assert(sz == 5);
    assert(memcmp(data2, (uint8_t const*) "Hello", sz) == 0);
    free(data2);

    connection_add_to_recv_buffer(conn, (uint8_t const*) "Hello\nWorld\ntest", 16);
    sz = connection_extract_line_from_recv_buffer(conn, &data2);
    assert(sz == 6);
    assert(memcmp(data2, (uint8_t const*) "Hello\n", sz) == 0);
    free(data2);

    sz = connection_extract_line_from_recv_buffer(conn, &data2);
    assert(sz == 6);
    assert(memcmp(data2, (uint8_t const*) "World\n", sz) == 0);
    free(data2);

    sz = connection_extract_line_from_recv_buffer(conn, &data2);
    assert(sz == 0);

    connection_destroy(conn);
}

//
// MAIN
//

int main()
{
    logs_verbose = true;

    // test_watchdog();
    test_connection();
    // test_connection_pool(SINGLE_THREADED);
    // test_connection_pool(3);

    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
