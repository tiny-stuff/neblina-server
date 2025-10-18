#ifndef NEBLINA_OS_H
#define NEBLINA_OS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define pid_t DWORD
#else
#  include <sys/types.h>
#endif

#ifdef _WIN32
#  define PID_NOT_RUNNING ((pid_t) 2147483646)
#else
#  define PID_NOT_RUNNING ((pid_t) -1)
#endif

void os_handle_ctrl_c();
void os_sleep_ms(size_t ms);

pid_t os_start_service(const char* program, const char** args, size_t args_sz);
bool  os_process_still_running(pid_t pid, int* status);
void  os_kill(pid_t pid, bool immediate);

#endif //NEBLINA_OS_H
