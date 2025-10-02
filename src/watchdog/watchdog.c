#include "watchdog.h"

#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util/logs.h"
#include "os/os.h"

typedef struct {
    size_t program_idx;
    pid_t  pid;
    int    recent_attempts;
    time_t last_attempt;
} Task;

static Task*         tasks = NULL;
static size_t        n_tasks = 0;
static volatile bool watchdog_running = true;

#define MAX_ATTEMPS          10
#define SEC_TO_RESET_ATTEMPS 10
#define MS_BETWEEN_RETRIES   100

static void start_task_if_stopped(Task* task)
{
    if (task->pid == -1) {
        if (task->recent_attempts == MAX_ATTEMPS) {
            LOG("Giving up on service '%s'", task->service->name);
            ++task->recent_attempts;
        } else if (task->recent_attempts < MAX_ATTEMPS) {
            LOG("Starting service '%s' with (attempt %d)", task->service->name, task->recent_attempts);
            task->pid = os_start_service(task->service);
            if (task->pid == 0) {
                ERR("Could not start process '%s': %s", task->service->name, last_error);
                return;
            }
            LOG("Service '%s' started with pid %d", task->service->name, task->pid);
            time(&task->last_attempt);
            ++task->recent_attempts;
        }
    }
}

static void mark_task_as_terminated_if_dead(Task *task)
{
    int status;
    if (!os_process_still_running(task->pid, &status)) {
        LOG("Sevice process '%s' has died with status %d%s", task->service->name, status,
            status == NON_RECOVERABLE_ERROR ? " (non-recoverable)" : "");
        task->pid = PID_NOT_RUNNING;
        if (status == NON_RECOVERABLE_ERROR)
            task->recent_attempts = 10;
    }
}


void watchdog_start(WatchdogProgram const* programs, size_t programs_sz)
{
    tasks = calloc(programs_sz, sizeof(Task));

    // create list of services
    for (int i = 0; i < programs_sz; ++i) {
        tasks[n_tasks++] = (Task) {
            .program_idx = i,
            .pid = PID_NOT_RUNNING,
            .recent_attempts = 0,
        };
        time(&tasks[i].last_attempt);
    }

    // keep track of services, restart if down
    while (watchdog_running) {

        // start/restart stopped services
        for (size_t i = 0; i < n_tasks; ++i)
            start_task_if_stopped(&tasks[i]);

        // check if any services have died
        for (size_t i = 0; i < n_tasks; ++i)
            mark_task_as_terminated_if_dead(&tasks[i]);

        // reset recent attempts
        time_t now; time(&now);
        for (size_t i = 0; i < n_tasks; ++i)
            if (tasks[i].recent_attempts >= MAX_ATTEMPS && difftime(now, tasks[i].last_attempt) > SEC_TO_RESET_ATTEMPS)
                tasks[i].recent_attempts = 0;

        os_sleep_ms(MS_BETWEEN_RETRIES);
    }
}

void watchdog_stop()
{
    watchdog_running = false;
    // TODO - kill all children
}

WatchdogProgramState watchdog_program_state(size_t idx)
{
}