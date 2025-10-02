#include "os/window.h"

#include "common.h"

#define ESC "\x1B"

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
}

void window_close()
{
}