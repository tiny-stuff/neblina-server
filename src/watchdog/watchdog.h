#ifndef NEBLINA_SERVER_WATCHDOG_H
#define NEBLINA_SERVER_WATCHDOG_H

#include <stddef.h>

#ifdef _WIN32
#else
#  include <sys/types.h>
#endif

#define NON_RECOVERABLE_ERROR 57
#define PID_NOT_RUNNING ((pid_t) -1)

typedef struct {
    const char*  name;
    const char*  program;
    const char** args;
} WatchdogProgram;

void watchdog_start(WatchdogProgram const* programs, size_t programs_sz);
void watchdog_stop();

typedef enum { WPS_RUNNING, WPS_STOPPED, WPS_GAVE_UP } WatchdogProgramStatus;
typedef struct {
    WatchdogProgramStatus status;
    int                   attempts;
    pid_t                 pid;
} WatchdogProgramState;

WatchdogProgramState watchdog_program_state(size_t idx);

#endif //NEBLINA_SERVER_WATCHDOG_H
