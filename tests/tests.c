#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "os/os.h"
#include "util/logs.h"
#include "server/cpool/cpool.h"
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
// CONNECTION POOL TESTS
//

static void test_connection_pool(size_t n_threads)
{
    cpool_init(n_threads, NULL);

    cpool_finalize();
}

//
// MAIN
//

int main()
{
    logs_verbose = true;

    // test_watchdog();
    test_connection_pool(SINGLE_THREADED);
    // test_connection_pool(3);

    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
