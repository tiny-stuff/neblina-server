#include "doctest.h"

#include <stdlib.h>

extern "C" {
#include "watchdog/watchdog.h"
extern bool logs_enabled;
}

TEST_SUITE("Watchdog")
{
    TEST_CASE("Watchdog")
    {
        logs_enabled = false;

        WatchdogProgram programs[] = {
                { "error",   "./tests-error",          NULL, 0 },
                { "infloop", "./tests-infloop",        NULL, 0 },
                { "nonrec",  "./tests-nonrecoverable", NULL, 0 },
        };
        watchdog_init(programs, sizeof programs / sizeof programs[0]);

        for (size_t i = 0; i < 5; ++i) {
            watchdog_step();
            // os_sleep_ms(100);
        }

        WatchdogProgramState error_state = watchdog_program_state(0);
        CHECK(error_state.attempts > 1);

        WatchdogProgramState infloop_state = watchdog_program_state(1);
        CHECK(infloop_state.status == WPS_RUNNING);
        CHECK(infloop_state.attempts == 1);
        CHECK(infloop_state.pid != PID_NOT_RUNNING);

        WatchdogProgramState nonrec_state = watchdog_program_state(2);
        CHECK(nonrec_state.status == WPS_GAVE_UP);
        CHECK(nonrec_state.attempts == 11);
        CHECK(nonrec_state.pid == PID_NOT_RUNNING);

        CHECK(os_process_still_running(infloop_state.pid, NULL));

        watchdog_finalize(true);

        CHECK(!os_process_still_running(error_state.pid, NULL));
        CHECK(!os_process_still_running(infloop_state.pid, NULL));
        CHECK(!os_process_still_running(nonrec_state.pid, NULL));

        logs_enabled = true;
    }
}