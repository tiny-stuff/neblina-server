#include "os/os.h"

#include <signal.h>
#include <string.h>

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

DWORD os_start_service(const char* program, const char** args, size_t args_sz)
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

    char cmd_line[1024] = {0}; char* s = cmd_line;
    s = strcat(s, program);
    s = strcat(s, " ");
    for (size_t i = 0; i < args_sz; ++i) {
        s = strcat(s, args[i]);
        s = strcat(s, " ");
    }

    DBG("Command line: %s\n", cmd_line);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    BOOL ok = CreateProcessA(
        program,             // application
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

void os_kill(DWORD pid)
{
    // Open the process with rights to terminate it
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        DWORD err = GetLastError();
        ERR("OpenProcess failed (PID=%lu), error=%lu", pid, err);
        return;
    }

    // Terminate the process
    if (!TerminateProcess(hProcess, 1)) {
        DWORD err = GetLastError();
        ERR("TerminateProcess failed (PID=%lu), error=%lu", pid, err);
        CloseHandle(hProcess);
        return;
    }

    CloseHandle(hProcess);
}