#include <assert.h>
#include <stdio.h>

#include "os/os.h"
#include "util/logs.h"
#include "watchdog/watchdog.h"

const char* service = "tests";

static void test_watchdog()
{
    WatchdogProgram programs[] = {
            { "infloop", "tests/infloop", NULL, 0},
    };
    watchdog_init(programs, sizeof programs / sizeof programs[0]);

    watchdog_step();
    watchdog_step();

    WatchdogProgramState infloop_state = watchdog_program_state(0);
    assert(infloop_state.status == WPS_RUNNING);
    assert(infloop_state.attempts == 1);
    assert(infloop_state.pid != -1);

    assert(os_process_still_running(infloop_state.pid, NULL));

    watchdog_finalize();

    os_sleep_ms(100);
    assert(!os_process_still_running(infloop_state.pid, NULL));
}

int main()
{
    logs_verbose = true;

    test_watchdog();

    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
