#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os/os.h"
#include "util/logs.h"
#include "server/commbuf.h"
#include "watchdog/watchdog.h"
#include "client/tcpclient.h"

const char* service = "tests";

//
// WATCHDOG TESTS
//

static void test_watchdog()
{
    WatchdogProgram programs[] = {
            { "error",   "./tests-error",          NULL, 0 },
            { "infloop", "./tests-infloop",        NULL, 0 },
            { "nonrec",  "./tests-nonrecoverable", NULL, 0 },
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

static void test_commbuf()
{
    CommunicationBuffer* conn = commbuf_create();

    // send buffer

    commbuf_add_to_send_buffer(conn, (uint8_t const*) "Hello", 5);
    commbuf_add_to_send_buffer(conn, (uint8_t const*) "World", 5);

    size_t sz;
    uint8_t const* data = commbuf_send_buffer(conn, &sz);
    assert(sz == 10);
    assert(memcmp(data, "HelloWorld", sz) == 0);

    commbuf_clear_send_buffer(conn);

    commbuf_send_buffer(conn, &sz);
    assert(sz == 0);

    commbuf_add_to_send_buffer(conn, (uint8_t const*) "Hello", 5);
    data = commbuf_send_buffer(conn, &sz);
    assert(sz == 5);
    assert(memcmp(data, "Hello", sz) == 0);

    // recv buffer

    commbuf_add_to_recv_buffer(conn, (uint8_t const*) "Hello", 5);
    uint8_t* data2;
    sz = commbuf_extract_from_recv_buffer(conn, &data2);
    assert(sz == 5);
    assert(memcmp(data2, (uint8_t const*) "Hello", sz) == 0);
    free(data2);

    char* data3;
    commbuf_add_to_recv_buffer(conn, (uint8_t const*) "Hello\nWorld\ntest", 16);
    sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
    assert(sz == 6);
    assert(memcmp(data3, (uint8_t const*) "Hello\n", sz) == 0);
    free(data3);

    sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
    assert(sz == 6);
    assert(memcmp(data3, (uint8_t const*) "World\n", sz) == 0);
    free(data3);

    sz = commbuf_extract_line_from_recv_buffer(conn, &data3, "\n");
    assert(sz == 0);

    commbuf_destroy(conn);
}

//
// PARROT (test TCP server and service)
//

static void test_parrot()
{
    // pid_t parrot_pid = os_start_service("./parrot-test", NULL, 0);
    // os_sleep_ms(100);

    TCPClient* t = tcpclient_create("localhost", 23456);
    assert(tcpclient_send_text(t, "hello") == 5);

    char resp[6] = {0};
    tcpclient_recv_spinlock(t, (uint8_t *) resp, 5);
    assert(strcmp(resp, "hello") == 0);

    tcpclient_destroy(t);
    // os_kill(parrot_pid);
}

//
// MAIN
//

int main()
{
    socket_init();

    logs_verbose = true;

    test_commbuf();
    // test_watchdog();
    test_parrot();

    socket_finalize();

    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
