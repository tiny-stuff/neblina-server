#include "logs.h"

#include "common.h"
#include "os/window.h"

void DBG(const char* fmt, ...)
{
    if (!args.verbose)
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
