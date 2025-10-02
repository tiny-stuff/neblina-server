#include "watchdog.h"

#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#include "util/logs.h"
#include "os/os.h"
#include "util/error.h"

typedef struct {
    size_t program_idx;
    pid_t  pid;
    int    recent_attempts;
    time_t last_attempt;
} Task;

static WatchdogProgram const* programs;
static size_t programs_sz;
static Task*         tasks = NULL;
static size_t        n_tasks = 0;

#define MAX_ATTEMPS          10
#define SEC_TO_RESET_ATTEMPS 10
#define MS_BETWEEN_RETRIES   100

static void start_task_if_stopped(Task* task)
{
    if (task->pid == -1) {
        if (task->recent_attempts == MAX_ATTEMPS) {
            LOG("Giving up on service '%s'", programs[task->program_idx].name);
            ++task->recent_attempts;
        } else if (task->recent_attempts < MAX_ATTEMPS) {
            LOG("Starting service '%s' with (attempt %d)", programs[task->program_idx].name, task->recent_attempts);
            task->pid = os_start_service(programs[task->program_idx].program, programs[task->program_idx].args, programs[task->program_idx].args_sz);
            if (task->pid == 0) {
                ERR("Could not start process '%s': %s", programs[task->program_idx].name, n_error(errno));
                return;
            }
            LOG("Service '%s' started with pid %d", programs[task->program_idx].name, task->pid);
            time(&task->last_attempt);
            ++task->recent_attempts;
        }
    }
}

static void mark_task_as_terminated_if_dead(Task *task)
{
    int status;
    if (!os_process_still_running(task->pid, &status)) {
        LOG("Sevice process '%s' has died with status %d%s", programs[task->program_idx].name, status,
            status == NON_RECOVERABLE_ERROR ? " (non-recoverable)" : "");
        task->pid = PID_NOT_RUNNING;
        if (status == NON_RECOVERABLE_ERROR)
            task->recent_attempts = 10;
    }
}


void watchdog_init(WatchdogProgram const* programs_, size_t programs_sz_)
{
    programs = programs_;
    programs_sz = programs_sz_;
    tasks = calloc(programs_sz, sizeof(Task));

    // create list of services
    for (size_t i = 0; i < programs_sz; ++i) {
        tasks[n_tasks++] = (Task) {
            .program_idx = i,
            .pid = PID_NOT_RUNNING,
            .recent_attempts = 0,
        };
        time(&tasks[i].last_attempt);
    }
}

void watchdog_step()
{
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

void watchdog_finalize()
{
    for (size_t i = 0; i < n_tasks; ++i)
        if (tasks[i].pid != PID_NOT_RUNNING)
            os_kill(tasks[i].pid);
    free(tasks);
}

WatchdogProgramState watchdog_program_state(size_t idx)
{
    WatchdogProgramState state;
    state.attempts = tasks[idx].recent_attempts;
    state.pid = tasks[idx].pid;
    if (state.pid == PID_NOT_RUNNING)
        state.status = WPS_STOPPED;
    else if (tasks[idx].recent_attempts >= MAX_ATTEMPS)
        state.status = WPS_GAVE_UP;
    else
        state.status = WPS_RUNNING;
    return state;
}
