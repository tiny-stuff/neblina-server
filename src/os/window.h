#ifndef NEBLINA_WINDOW_H
#define NEBLINA_WINDOW_H

#include <stdarg.h>

#ifndef _MSC_VER
#  define ATTR_PRINTF(a, b) __attribute__ ((format (printf, a, b)))
#else
#  define ATTR_PRINTF(a, b)
#endif

void vnprintf(const char *restrict fmt, va_list ap);
void vnprintf_error(const char *restrict fmt, va_list ap);

static inline void nprintf(const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vnprintf(fmt, ap);
    va_end(ap);
}

static inline void nprintf_error(const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vnprintf_error(fmt, ap);
    va_end(ap);
}

void window_init();
void window_close();

#endif //NEBLINA_WINDOW_H
