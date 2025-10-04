#ifndef NEBLINA_SERVER_CPOOL_H
#define NEBLINA_SERVER_CPOOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../server.h"
#include "../connection.h"

// This is a thread pool that receives tasks, put the tasks in a queue, and then process them.
// There's a lock number which prevents two tasks with the same lock number to be executed at the same time.

#define SINGLE_THREADED 0

void cpool_init(size_t n_threads, Server* server);
void cpool_finalize();

void cpool_add_connection(Connection* connection);
void cpool_remove_connection(Connection* connection);

void cpool_flush_connection(Connection* connection);

#endif //NEBLINA_SERVER_CPOOL_H
