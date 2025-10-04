#ifndef NEBLINA_SERVER_CPOOL_H
#define NEBLINA_SERVER_CPOOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct CPool CPool;
typedef struct Server Server;
typedef struct Connection Connection;

// This is a thread pool that receives tasks, put the tasks in a queue, and then process them.
// There's a lock number which prevents two tasks with the same lock number to be executed at the same time.

CPool* cpool_create(size_t n_threads, Server* server);
void cpool_destroy(CPool* cpool);

void cpool_add_connection(CPool* cpool, Connection* connection);
void cpool_remove_connection(CPool* cpool, Connection* connection);

void cpool_flush_connection(CPool* cpool, Connection* connection);

#endif //NEBLINA_SERVER_CPOOL_H
