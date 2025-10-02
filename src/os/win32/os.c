#include "os/os.h"

#include "common.h"

#include <signal.h>

static HANDLE g_job = NULL;   // Global job handle to tie children to parent

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
    Sleep((DWORD) ms);
}

DWORD os_start_service(ConfigService const* service)
{
    if (!g_job) {

        // create a Job Object that terminates all children when parent dies
        g_job = CreateJobObject(NULL, NULL);
        if (!g_job) {
            THROW("CreateJobObject failed : % lu", GetLastError());
        }
        else {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(g_job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
        }
    }

    char cmd_line[1024] = { 0 };
    snprintf(cmd_line, sizeof(cmd_line),
        "\"%s\" -D \"%s\" -s %s -P %d -c %d%s%s",
        args.program_name,
        args.data_dir,
        service->name,
        service->port,
        service->logging_color,
        service->open_to_world ? " -w" : "",
        args.verbose ? " -v" : ""
    );

    DBG("Command line: %s\n", cmd_line);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    BOOL ok = CreateProcessA(
        args.program_name,   // application
        cmd_line,            // command line
        NULL,                // proc security
        NULL,                // thread security
        TRUE,                // handles inheritable
        CREATE_SUSPENDED,    // so we can assign to job before running
        NULL,                // env
        NULL,                // cwd
        &si,
        &pi
    );

    if (!ok)
        THROW("CreateProcess failed: %lu\n", GetLastError());

    if (g_job) {
        if (!AssignProcessToJobObject(g_job, pi.hProcess)) {
            fprintf(stderr, "AssignProcessToJobObject failed: %lu\n", GetLastError());
        }
    }

    // Now let child run
    ResumeThread(pi.hThread);

    return pi.dwProcessId;
}

bool os_process_still_running(DWORD dwProcessId, int* status)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        *status = 1;
        return false;
	}

    DWORD code;
    if (!GetExitCodeProcess(hProcess, &code)) {
        return false;
    }

    if (status) {
        *status = code;
    }

    return code == STILL_ACTIVE;
}
