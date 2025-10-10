#ifndef NEBLINA_SERVER_CPOOL_H
#define NEBLINA_SERVER_CPOOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct SessionPool SessionPool;
typedef struct Server Server;
typedef struct Session Session;

// This is a thread pool that receives tasks, put the tasks in a queue, and then process them.
// There's a lock number which prevents two tasks with the same lock number to be executed at the same time.

SessionPool* spool_create(size_t n_threads, Server* server);
void spool_destroy(SessionPool* cpool);

void spool_add_session(SessionPool* cpool, Session* session);
void spool_remove_session(SessionPool* cpool, Session* session);

void spool_flush_session(SessionPool* cpool, Session* session);

#endif //NEBLINA_SERVER_CPOOL_H
