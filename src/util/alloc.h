#ifndef NEBLINA_ALLOC_H
#define NEBLINA_ALLOC_H

#include <stddef.h>

void* MALLOC(size_t size);
void* CALLOC(size_t n, size_t size);
void* REALLOC(void *p, size_t size);

#endif //NEBLINA_ALLOC_H
