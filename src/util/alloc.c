#include "alloc.h"

#include <stdlib.h>
#include <stdio.h>

void* check(void* m)
{
    if (!m) {
        fprintf(stderr, "Memory exhausted.\n");
        exit(EXIT_FAILURE);
    }
    return m;
}

void* MALLOC(size_t size)
{
    void* m = malloc(size);
    return check(m);
}

void* CALLOC(size_t n, size_t size)
{
    void* m = calloc(n, size);
    return check(m);
}

void* REALLOC(void *p, size_t size)
{
    void* m = realloc(p, size);
    return check(m);
}
