#include "future.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

#include "alloc.h"

struct ThreadData {
    FutureThread f;
    Future*      future;
    void*        data;
};

typedef struct Future {
    pthread_t          thread;
    pthread_mutex_t    mutex;
    FutureStatus       status;
    void*              result;
    struct ThreadData* dt;
} Future;

static void* run_thread(void* data)
{
    struct ThreadData* dt = data;
    void* r = dt->f(dt->future, dt->data);   // execute user code
    pthread_mutex_lock(&dt->future->mutex);
    if (dt->future->status != FU_ERROR) {
        dt->future->status = FU_SUCCESS;
        dt->future->result = r;
    } else {
        r = dt->future->result;
    }
    pthread_mutex_unlock(&dt->future->mutex);
    return r;
}

static void future_initialize(Future* future, FutureThread future_thread, void* data)
{
    memset(future, 0, sizeof(*future));
    pthread_mutex_init(&future->mutex, NULL);
    future->status = FU_RUNNING;
    future->result = NULL;

    future->dt = MALLOC(sizeof *future->dt);
    future->dt->f = future_thread;
    future->dt->future = future;
    future->dt->data = data;

    pthread_create(&future->thread, NULL, run_thread, future->dt);
}

static void future_finalize(Future* future)
{
    free(future->dt);
    future_await(future, NULL);
    pthread_mutex_destroy(&future->mutex);
}

Future* future_create(FutureThread future_thread, void* data)
{
    Future* future = MALLOC(sizeof(Future));
    if (future == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    future_initialize(future, future_thread, data);
    return future;
}

void future_destroy(Future* future)
{
    future_finalize(future);
    free(future);
}

FutureStatus future_await(Future* future, void** result)
{
    void* tmp_result;
    pthread_join(future->thread, &tmp_result);
    if (future->status == FU_ERROR) {
        if (result)
            *result = future->result;
        return FU_ERROR;
    } else if (future->status == FU_SUCCESS) {
        if (result)
            *result = tmp_result;
        return FU_SUCCESS;
    }

    abort();  // should not reach here
}

FutureStatus future_status(Future* future)
{
    pthread_mutex_lock(&future->mutex);
    FutureStatus status = future->status;
    pthread_mutex_unlock(&future->mutex);
    return status;
}

void future_notify_error(Future* future, void* result)
{
    pthread_mutex_lock(&future->mutex);
    future->status = FU_ERROR;
    future->result = result;
    pthread_mutex_unlock(&future->mutex);
}
