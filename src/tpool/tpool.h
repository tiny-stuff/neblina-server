#ifndef NEBLINA_SERVER_TPOOL_H
#define NEBLINA_SERVER_TPOOL_H

#include <stdbool.h>

// This is a thread pool that receives tasks, put the tasks in a queue, and then process them.
// There's a lock number which prevents two tasks with the same lock number to be executed at the same time.

typedef void(*TPoolTask)(int idx, void* data);

void tpool_init(bool multithreaded);
void tpool_finalize();

void tpool_add_task(TPoolTask task, int lane, void* data);

#endif //NEBLINA_SERVER_TPOOL_H
