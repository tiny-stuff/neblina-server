#include "tpool.h"

#include <stdlib.h>
#include <stdint.h>
#include <threads.h>

#include "util/error.h"
#include "uthash/uthash.h"
#include "uthash/utlist.h"

// thread management
static size_t  n_threads = 0;
static thrd_t* threads = NULL;
static bool*   thread_running = NULL;

// store buffers
typedef struct Buffer {
    int            fd;
    char*          buffer;
    size_t         buffer_sz;
    mtx_t          mutex;
    UT_hash_handle hh;
} Buffer;

static Buffer* buffers = NULL;

static int thread_function(void* arg)
{
    intptr_t n = (intptr_t) arg;

    DBG("Creating thread %zi", n);

    while (thread_running[n])
        thrd_sleep(&(struct timespec){ .tv_nsec = 10 * 1000 * 100 }, NULL);

    return 0;
}

void tpool_init(size_t n_threads_)
{
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

void tpool_finalize()
{
    // end threads
    for (size_t i = 0; i < n_threads; ++i)
        thread_running[i] = false;
    for (size_t i = 0; i < n_threads; ++i) {
        thrd_join(threads[i], NULL);
        DBG("Thread %zi finalized", i);
    }

    // cleanup
    free(threads);
    free(thread_running);
}

void tpool_add_task(TPoolTask task, int fd, uint8_t const* data, size_t data_sz)
{
    if (n_threads == SINGLE_THREADED) {
        task(fd, data, data_sz);

    } else {

        // find/create index in hash table
        Buffer* buffer;
        HASH_FIND_INT(buffers, &fd, buffer);
        if (buffer == NULL) {
            buffer = calloc(1, sizeof(Buffer));
            buffer->fd = fd;
            HASH_ADD_INT(buffers, fd, buffer);
        }

        // lock mutex and add to buffer
        mtx_lock(&buffer->mutex);
        buffer->buffer = realloc(buffer->buffer, buffer->buffer_sz + data_sz);
        memcpy(&buffer->buffer[buffer->buffer_sz], buffer->buffer, data_sz);
        buffer->buffer_sz += data_sz;
        mtx_unlock(&buffer->mutex);
    }
}