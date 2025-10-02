#include "os/os.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

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

/*
pid_t os_start_service(ConfigService const* service)
{
    // arguments
    size_t n = 0;
    const char* argv[15];
    argv[n++] = args.program_name;
    argv[n++] = "-D";
    argv[n++] = args.data_dir;
    argv[n++] = "-s";
    argv[n++] = service->name;
    argv[n++] = "-P";
    char port[10]; snprintf(port, sizeof(port), "%d", service->port);
    argv[n++] = port;
    char color[5]; snprintf(color, sizeof(color), "%d", service->logging_color);
    argv[n++] = "-c";
    argv[n++] = color;
    if (service->open_to_world)
        argv[n++] = "-w";
    if (args.verbose)
        argv[n++] = "-v";

    // print command line
    if (args.verbose) {
        char cmd_line[1024] = {0}; char* s = cmd_line;
        for (size_t i = 0; i < n; ++i) {
            s = strcat(s, argv[i]);
            s = strcat(s, " ");
        }
        DBG("Command line: %s", cmd_line);
    }

    pid_t pid = fork();
    if (pid == 0) {  // child process
        argv[n] = NULL;
        execvp(args.program_name, (char *const *) argv);
        FATAL_NON_RECOVERABLE("execvp failed when starting a new service: %s", strerror(errno));
    } else if (pid > 0) {
        setpgid(pid, getpid());  // register child
    } else {
        ERR("fork error: %s", strerror(errno));
    }

    return pid;
}
*/

bool os_process_still_running(pid_t pid, int* status)
{
    return waitpid(pid, status, WNOHANG) != pid;
}