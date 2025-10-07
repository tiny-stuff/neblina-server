#ifndef NEBLINA_SERVER_WATCHDOG_H
#define NEBLINA_SERVER_WATCHDOG_H

#include <stddef.h>

#include "os.h"

#define PID_NOT_RUNNING ((pid_t) -1)

typedef struct {
    const char*  name;
    const char*  program;
    const char** args;
    size_t       args_sz;
} WatchdogProgram;

void watchdog_init(WatchdogProgram const* programs, size_t programs_sz);
void watchdog_step();
void watchdog_finalize();

typedef enum { WPS_RUNNING, WPS_STOPPED, WPS_GAVE_UP } WatchdogProgramStatus;
typedef struct {
    WatchdogProgramStatus status;
    int                   attempts;
    pid_t                 pid;
} WatchdogProgramState;

WatchdogProgramState watchdog_program_state(size_t idx);

#endif //NEBLINA_SERVER_WATCHDOG_H
