#ifndef NEBLINA_LOG_H
#define NEBLINA_LOG_H

#ifndef _MSC_VER
#  define ATTR_PRINTF(a, b) __attribute__ ((format (printf, a, b)))
#else
#  define ATTR_PRINTF(a, b)
#endif

void DBG(const char* fmt, ...) ATTR_PRINTF(1, 2);
void LOG(const char* fmt, ...) ATTR_PRINTF(1, 2);
void ERR(const char* fmt, ...) ATTR_PRINTF(1, 2);

#endif //NEBLINA_LOG_H
