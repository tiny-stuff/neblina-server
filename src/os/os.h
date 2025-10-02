#ifndef NEBLINA_OS_H
#define NEBLINA_OS_H

#include <stddef.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define pid_t DWORD
#else
#  include <sys/types.h>
#endif

void os_handle_ctrl_c();
void os_sleep_ms(size_t ms);

// pid_t os_start_service(ConfigService const* service);
bool  os_process_still_running(pid_t pid, int* status);

#endif //NEBLINA_OS_H
