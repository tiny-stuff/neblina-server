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

// task management
static mtx_t   task_queue_mutex;

typedef struct Task {
    TPoolTask        task;
    void*            args;
    struct Task* prev;
    struct Task* next;
} Task;

typedef struct LaneQueue {
    int            lane;
    Task*          queue;
    UT_hash_handle hh;
} LaneQueue;

static LaneQueue* queues_per_lane;

// lane ready queue

typedef struct LaneReadyQueue {
    int                    line;
    struct LaneReadyQueue* prev;
    struct LaneReadyQueue* next;
} LaneReadyQueue;



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
    for (size_t i = 0; i < n_threads; ++i)
        thread_running[i] = false;
    for (size_t i = 0; i < n_threads; ++i) {
        thrd_join(threads[i], NULL);
        DBG("Thread %zi finalized", i);
    }
    free(threads);
    free(thread_running);
}

void tpool_add_task(TPoolTask task, int lane, void* data)
{
    if (n_threads == SINGLE_THREADED) {
        task(lane, data);
    } else {
        mtx_lock(&task_queue_mutex);

        // find or create queue per lane
        LaneQueue* q;
        HASH_FIND_INT(queues_per_lane, &lane, q);
        if (!q) {
            q = malloc(sizeof(LaneQueue));
            q->lane = lane;
            q->queue = NULL;
            HASH_ADD_INT(queues_per_lane, lane, q);
        }

        // add task to the queue
        Task* ntask = malloc(sizeof(Task));
        ntask->task = task;
        ntask->args = data;
        DL_APPEND(q->queue, ntask);

        mtx_unlock(&task_queue_mutex);
    }
}