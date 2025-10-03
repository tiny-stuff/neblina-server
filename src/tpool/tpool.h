#ifndef NEBLINA_SERVER_TPOOL_H
#define NEBLINA_SERVER_TPOOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// This is a thread pool that receives tasks, put the tasks in a queue, and then process them.
// There's a lock number which prevents two tasks with the same lock number to be executed at the same time.

typedef void(*TPoolTask)(int fd, uint8_t const* data, size_t data_sz);

#define SINGLE_THREADED 0

void tpool_init(size_t n_threads);
void tpool_finalize();

void tpool_add_task(TPoolTask task, int fd, uint8_t const* data, size_t data_sz);

#endif //NEBLINA_SERVER_TPOOL_H
