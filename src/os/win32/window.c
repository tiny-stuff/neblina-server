#include "os/window.h"

#include "common.h"

#include <windows.h>

#define ESC "\x1B"

// TODO - move this to a logging window

void vnprintf(const char *restrict fmt, va_list ap)
{
    printf(ESC "[0;%dm%-13s: ", args.logging_color, args.service ? args.service : "main");
    vfprintf(stdout, fmt, ap);
    printf(ESC "[0m\n");
}

void vnprintf_error(const char *restrict fmt, va_list ap)
{
    fprintf(stderr, "%s: \n", args.service);
    vfprintf(stderr, fmt, ap);
}

void window_init()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleTitleA("neblina");
}

void window_close()
{
}
