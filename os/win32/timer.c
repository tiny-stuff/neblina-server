#include "timer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

typedef struct Timer {
    LARGE_INTEGER freq;
    LARGE_INTEGER start_qpc;
} Timer;

static void timer_initialize(Timer* timer)
{
    memset(timer, 0, sizeof(*timer));
    QueryPerformanceFrequency(&timer->freq);
    QueryPerformanceCounter(&timer->start_qpc);
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
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    uint64_t elapsed = (uint64_t)((now.QuadPart - timer->start_qpc.QuadPart) * 1000ULL / timer->freq.QuadPart);
    return elapsed;
}

