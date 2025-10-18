#include "timer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <sys/time.h>

typedef struct Timer {
    uint64_t start;
} Timer;

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

static void timer_initialize(Timer* timer)
{
    memset(timer, 0, sizeof(*timer));
    timer->start = now_ms();
}

static void timer_finalize(Timer* timer)
{
    (void) timer;
}

Timer* timer_create_()
{
    Timer* timer = malloc(sizeof(Timer));
    if (timer == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    timer_initialize(timer);
    return timer;
}

void timer_destroy(Timer* timer)
{
    timer_finalize(timer);
    free(timer);
}

uint64_t timer_current_ms(Timer* timer)
{
    return now_ms() - timer->start;
}
