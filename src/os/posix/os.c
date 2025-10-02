#include "os/os.h"

#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "util/error.h"
#include "util/logs.h"

extern bool termination_requested;

void handle_sigint(int signum)
{
    (void) signum;
    termination_requested = true;
}

void os_handle_ctrl_c()
{
    // signal to catch CTRL+C and exit gracefully
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);
}

void os_sleep_ms(size_t ms)
{
    usleep(ms * 1000);
}

pid_t os_start_service(const char* program, char* const* args)
{
    // print command line
    if (logs_verbose) {
        char cmd_line[1024] = {0}; char* s = cmd_line;
        for (char* const* str = args; *str; ++str) {
            s = strcat(s, (const char *) str);
            s = strcat(s, " ");
        }
        DBG("Command line: %s", cmd_line);
    }

    pid_t pid = fork();
    if (pid == 0) {  // child process
        execvp(program, args);
        FATAL_NON_RECOVERABLE("execvp failed when starting a new service: %s", strerror(errno));
    } else if (pid > 0) {
        setpgid(pid, getpid());  // register child
    } else {
        ERR("fork error: %s", strerror(errno));
    }

    return pid;
}

bool os_process_still_running(pid_t pid, int* status)
{
    return waitpid(pid, status, WNOHANG) != pid;
}