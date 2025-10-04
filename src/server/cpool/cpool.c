#include "cpool.h"

#include <stdlib.h>
#include <stdint.h>
#include <threads.h>

#include "util/error.h"
#include "uthash/uthash.h"
#include "uthash/utlist.h"

#include "../server.h"
#include "../connection.h"

static Server* server;

// thread management
static size_t  n_threads = 0;
static thrd_t* threads = NULL;
static bool*   thread_running = NULL;

static int thread_function(void* arg)
{
    intptr_t n = (intptr_t) arg;

    DBG("Creating thread %zi", n);

    while (thread_running[n])
        thrd_sleep(&(struct timespec){ .tv_nsec = 1 * 1000 * 1000 }, NULL);

    return 0;
}

void cpool_init(size_t n_threads_, Server* server_)
{
    server = server_;

    // create threads
    n_threads = n_threads_;
    if (n_threads != SINGLE_THREADED) {

        thread_running = calloc(n_threads, sizeof thread_running[0]);
        threads = calloc(n_threads, sizeof threads[0]);
        for (size_t i = 0; i < n_threads; ++i) {
            thread_running[i] = true;
            if (thrd_create(&threads[i], thread_function, (void *) i) != thrd_success)
                FATAL_NON_RECOVERABLE("Unable to create thread.");
        }
    }
}

void cpool_finalize()
{
    // end threads
    for (size_t i = 0; i < n_threads; ++i)
        thread_running[i] = false;
    for (size_t i = 0; i < n_threads; ++i) {
        thrd_join(threads[i], NULL);
        DBG("Thread %zu finalized", i);
    }

    // cleanup
    free(threads);
    free(thread_running);
}

void cpool_add_connection(Connection* connection)
{
    if (n_threads != SINGLE_THREADED) {
        // TODO - add connection to the least populated thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    }
}

void cpool_remove_connection(Connection* connection)
{
    if (n_threads != SINGLE_THREADED) {
        // TODO - remove connection from the thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    }
}

void cpool_flush_connection(Connection* connection)
{
    if (n_threads != SINGLE_THREADED) {
        // TODO - mark connection as available for flushing
        // TODO - wake up thread
        FATAL_NON_RECOVERABLE("Not implemented yet");
    } else {
        // server_flush_connection(server, connection);
    }
}