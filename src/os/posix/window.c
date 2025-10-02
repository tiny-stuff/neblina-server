#include "os/window.h"

#include <stdarg.h>
#include <stdio.h>

#define ESC "\x1B"

extern int         logging_color;
extern const char* service;

void vnprintf(const char *restrict fmt, va_list ap)
{
    printf(ESC "[0;%dm%-13s: ", logging_color, service);
    vfprintf(stdout, fmt, ap);
    printf(ESC "[0m\n");
}

void vnprintf_error(const char *restrict fmt, va_list ap)
{
    fprintf(stderr, "%s: \n", service);
    vfprintf(stderr, fmt, ap);
}

void window_init()
{
}

void window_close()
{
}