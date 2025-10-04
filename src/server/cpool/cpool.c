#include "cpool.h"

#include <stdlib.h>
#include <stdint.h>
#include <threads.h>

#include "util/error.h"
#include "uthash/uthash.h"
#include "uthash/utlist.h"

#include "../server.h"
#include "../connection.h"

typedef struct ThreadArgs {
    struct CPool* cpool;
    size_t        thread_n;
} ThreadArgs;

typedef struct CPool {
    Server*     server;
    size_t      n_threads;
    thrd_t*     threads;
    bool*       thread_running;
    ThreadArgs* args;
} CPool;

static int thread_function(void* arg)
{
    ThreadArgs* args = arg;

    DBG("Creating thread %zu", args->thread_n);

    while (args->cpool->thread_running[args->thread_n])
        thrd_sleep(&(struct timespec){ .tv_nsec = 1 * 1000 * 1000 }, NULL);

    return 0;
}

CPool* cpool_create(size_t n_threads, Server* server)
{
    CPool* cpool = calloc(1, sizeof(CPool));
    cpool->server = server;

    // create threads
    cpool->n_threads = n_threads;
    if (n_threads != SINGLE_THREADED) {

        cpool->thread_running = calloc(n_threads, sizeof cpool->thread_running[0]);
        cpool->threads = calloc(n_threads, sizeof cpool->threads[0]);
        cpool->args = calloc(n_threads, sizeof cpool->args[0]);

        for (size_t i = 0; i < n_threads; ++i) {
            cpool->thread_running[i] = true;
            cpool->args[i] = (ThreadArgs) { .cpool = cpool, .thread_n = i };
            if (thrd_create(&cpool->threads[i], thread_function, &cpool->args[i]) != thrd_success)
                FATAL_NON_RECOVERABLE("Unable to create thread.");
        }
    }

    return cpool;
}

void cpool_destroy(CPool* cpool)
{
    // end threads
    for (size_t i = 0; i < cpool->n_threads; ++i)
        cpool->thread_running[i] = false;
    for (size_t i = 0; i < cpool->n_threads; ++i) {
        thrd_join(cpool->threads[i], NULL);
        DBG("Thread %zu finalized", i);
    }

    // cleanup
    free(cpool->threads);
    free(cpool->thread_running);
    free(cpool->args);
}

void cpool_add_connection(CPool* cpool, Connection* connection)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        // TODO - add connection to the least populated thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    }
}

void cpool_remove_connection(CPool* cpool, Connection* connection)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        // TODO - remove connection from the thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    }
}

void cpool_flush_connection(CPool* cpool, Connection* connection)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        // TODO - mark connection as available for flushing
        // TODO - wake up thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    } else {
        server_flush_connection(cpool->server, connection);
    }
}