#include <assert.h>
#include <stdio.h>

#include "os/os.h"
#include "util/logs.h"
#include "tpool/tpool.h"
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
// THREAD POOL TESTS
//

volatile int order = 1;

static void task0(int lane, void* data) { (void) lane; int* array = data; array[0] = order++; os_sleep_ms(100); }
static void task1(int lane, void* data) { (void) lane; int* array = data; array[1] = order++; os_sleep_ms(100); }
static void task2(int lane, void* data) { (void) lane; int* array = data; array[2] = order++; os_sleep_ms(100); }

static void test_thread_pool(bool multithreaded)
{
    tpool_init(multithreaded);

    // the idea here is that tasks that don't have the same idx should not happen at the same time
    // so: it the code bellow should happen in this order: task0, task1, task2
    int array[3] = { 0, 0, 0 };
    tpool_add_task(task0, 1, array);
    tpool_add_task(task2, 1, array);
    os_sleep_ms(30);
    tpool_add_task(task1, 2, array);

    if (multithreaded) {
        assert(array[0] == 1);
        assert(array[1] == 2);
        assert(array[2] == 3);
    } else {
        assert(array[0] == 1);
        assert(array[1] == 3);
        assert(array[2] == 2);
    }

    tpool_finalize();
}

//
// MAIN
//

int main()
{
    logs_verbose = true;

    // test_watchdog();
    test_thread_pool(false);

    printf("\x1b[0;32mTests successful!\x1b[0m\n");
}
