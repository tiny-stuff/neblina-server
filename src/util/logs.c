#include "logs.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "os/window.h"

bool logs_verbose = false;
int  logging_color = 31;

void DBG(const char* fmt, ...)
{
    if (!logs_verbose)
        return;
    va_list ap;
    va_start(ap, fmt);
    vnprintf(fmt, ap);
    va_end(ap);
}

void LOG(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vnprintf(fmt, ap);
    va_end(ap);
}

void ERR(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vnprintf_error(fmt, ap);
    va_end(ap);
}
