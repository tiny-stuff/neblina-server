#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

typedef struct Timer Timer;

Timer* timer_create_();
void timer_destroy(Timer* timer);

uint64_t timer_current_ms(Timer* timer);

#endif
