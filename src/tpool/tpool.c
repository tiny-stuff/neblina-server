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
    uint8_t*       buffer;
    size_t         buffer_sz;
    mtx_t          mutex;
    UT_hash_handle hh;
} Buffer;

// fd queue
typedef struct FdQueue {
    int             fd;
    TPoolTask       task;
    struct FdQueue* prev;
    struct FdQueue* next;
} FdQueue;

static Buffer*  buffers = NULL;
static FdQueue* fd_queue = NULL;
static mtx_t    fd_queue_mutex;
static cnd_t    fd_condition_variable;

static int thread_function(void* arg)
{
    size_t n = (size_t)(intptr_t)arg;

    DBG("Creating thread %zu", n);

    for (;;) {
        mtx_lock(&fd_queue_mutex);
        while (fd_queue == NULL && thread_running[n]) {
            cnd_wait(&fd_condition_variable, &fd_queue_mutex);
        }

        /* if shutting down and no work, exit */
        if (!thread_running[n] && fd_queue == NULL) {
            mtx_unlock(&fd_queue_mutex);
            break;
        }

        /* dequeue front */
        int fd = -1;
        TPoolTask task = NULL;
        if (fd_queue != NULL) {
            fd = fd_queue->fd;
            task = fd_queue->task;
            DL_DELETE(fd_queue, fd_queue);
        }
        mtx_unlock(&fd_queue_mutex);

        if (fd == -1)
            continue;

        /* find buffer: protected by fd_queue_mutex in other side, but here
           we only read the hash — still need synchronization. We'll use
           the same fd_queue_mutex for consistency (lighter to re-lock briefly). */
        Buffer* buffer = NULL;
        mtx_lock(&fd_queue_mutex);
        HASH_FIND_INT(buffers, &fd, buffer);
        mtx_unlock(&fd_queue_mutex);

        if (!buffer)
            continue;

        mtx_lock(&buffer->mutex);
        if (task)
            task(fd, buffer->buffer, buffer->buffer_sz);
        buffer->buffer_sz = 0;
        mtx_unlock(&buffer->mutex);
    }

    return 0;
}


void tpool_init(size_t n_threads_)
{
    // create threads
    n_threads = n_threads_;
    if (n_threads != SINGLE_THREADED) {

        mtx_init(&fd_queue_mutex, mtx_plain);
        cnd_init(&fd_condition_variable);

        thread_running = calloc(n_threads, sizeof thread_running[0]);
        threads = calloc(n_threads, sizeof threads[0]);
        for (size_t i = 0; i < n_threads; ++i) {
            thread_running[i] = true;
            if (thrd_create(&threads[i], thread_function, (void *) i) != thrd_success)
                FATAL_NON_RECOVERABLE("Unable to create thread.");
        }

        cnd_broadcast(&fd_condition_variable);
    }
}

void tpool_finalize()
{
    // end threads
    for (size_t i = 0; i < n_threads; ++i)
        thread_running[i] = false;
    for (size_t i = 0; i < n_threads; ++i) {
        thrd_join(threads[i], NULL);
        DBG("Thread %zu finalized", i);
    }

    mtx_lock(&fd_queue_mutex);
    cnd_broadcast(&fd_condition_variable);
    mtx_unlock(&fd_queue_mutex);

    // cleanup
    free(threads);
    free(thread_running);
}

void tpool_add_task(TPoolTask task, int fd, uint8_t const* data, size_t data_sz)
{
    if (n_threads == SINGLE_THREADED) {
        task(fd, data, data_sz);
        return;
    }

    /* Lock queue mutex — also protects buffers hash to keep ordering consistent. */
    mtx_lock(&fd_queue_mutex);

    /* find/create buffer while holding fd_queue_mutex */
    Buffer* buffer = NULL;
    HASH_FIND_INT(buffers, &fd, buffer);
    if (buffer == NULL) {
        buffer = calloc(1, sizeof(Buffer));
        if (!buffer) {
            mtx_unlock(&fd_queue_mutex);
            FATAL_NON_RECOVERABLE("OOM");
        }
        buffer->fd = fd;
        mtx_init(&buffer->mutex, mtx_plain);
        HASH_ADD_INT(buffers, fd, buffer);
    }

    /* Now lock the buffer mutex (we hold fd_queue_mutex; worker will lock same order) */
    mtx_lock(&buffer->mutex);

    /* grow buffer — check realloc result */
    uint8_t* p = realloc(buffer->buffer, buffer->buffer_sz + data_sz);
    if (!p) {
        /* allocation failed — handle error. For now, we unlock and abort */
        mtx_unlock(&buffer->mutex);
        mtx_unlock(&fd_queue_mutex);
        FATAL_NON_RECOVERABLE("OOM");
    }
    buffer->buffer = p;
    memcpy(&buffer->buffer[buffer->buffer_sz], data, data_sz);
    buffer->buffer_sz += data_sz;
    mtx_unlock(&buffer->mutex);

    /* enqueue a task for this fd */
    FdQueue* item = calloc(1, sizeof(FdQueue));
    if (!item) {
        mtx_unlock(&fd_queue_mutex);
        FATAL_NON_RECOVERABLE("OOM");
    }
    item->fd = fd;
    item->task = task;
    DL_APPEND(fd_queue, item);

    /* signal one worker */
    cnd_signal(&fd_condition_variable);

    mtx_unlock(&fd_queue_mutex);
}