#include "os/window.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define ESC "\x1B"

#include <stdarg.h>
#include <stdio.h>

extern int         logging_color;
extern const char* service;

// TODO - move this to a logging window

void vnprintf(const char *restrict fmt, va_list ap)
{
    printf(ESC "[0;%dm%-13s: ", logging_color, service);
    vfprintf(stdout, fmt, ap);
    printf(ESC "[0m\n");
}

void vnprintf_error(const char *restrict fmt, va_list ap)
{
    fprintf(stderr, "%s: ", service);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

void window_init()
{
    // rename console
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
