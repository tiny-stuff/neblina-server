#include "os/os.h"

#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>

#include "util/error.h"
#include "util/logs.h"

bool termination_requested = false;

void handle_sigint(int signum)
{
    (void) signum;
    DBG("Termination requested");
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
    usleep((useconds_t) (ms * 1000));
}

pid_t os_start_service(const char* program, const char** args, size_t args_sz)
{
    // print command line
    if (logs_verbose) {
        char cmd_line[1024] = {0}; char* s = cmd_line;
        for (size_t i = 0; i < args_sz; ++i) {
            s = strcat(s, args[i]);
            s = strcat(s, " ");
        }
        DBG("Command line: %s %s", program, cmd_line);
    }

    char** pargs = calloc(args_sz + 2, sizeof (char *));
    pargs[0] = (char *) (uintptr_t) program;
    for (size_t i = 0; i < args_sz; ++i)
        pargs[i + 1] = (char *) (uintptr_t) args[i];

    pid_t pid = fork();
    if (pid == 0) {  // child process
        execvp(program, pargs);
        FATAL_NON_RECOVERABLE("execvp failed when starting a new service: %s", strerror(errno));
    } else if (pid > 0) {
        setpgid(pid, getpid());  // register child
        free(pargs);
    } else {
        ERR("fork error: %s", strerror(errno));
    }

    return pid;
}

bool os_process_still_running(pid_t pid, int* status)
{
    bool r = waitpid(pid, status, WNOHANG) != pid;
    if (status)
        *status = WEXITSTATUS(*status);
    return r;
}

void os_kill(pid_t pid)
{
    kill(pid, SIGINT);
}
