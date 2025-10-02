#include <assert.h>
#include <stdio.h>

#include "os/os.h"
#include "watchdog/watchdog.h"

bool termination_requested = false;

static void test_watchdog()
{
    WatchdogProgram programs[] = {
            { "infloop", "tests/watchdog/infloop", (const char*[]) { "infloop", NULL } },
    };
    watchdog_start(programs, sizeof programs / sizeof programs[0]);

    os_sleep_ms(200);

    WatchdogProgramState infloop_state = watchdog_program_state(0);
    assert(infloop_state.status == WPS_RUNNING);
    assert(infloop_state.attempts == 1);
    assert(infloop_state.pid != -1);

    assert(os_process_still_running(infloop_state.pid, NULL));

    watchdog_stop();

    os_sleep_ms(100);
    assert(!os_process_still_running(infloop_state.pid, NULL));
}

int main()
{
    test_watchdog();
    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
