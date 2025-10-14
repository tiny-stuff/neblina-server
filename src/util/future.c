#include "future.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

typedef struct Future {
    void* result;
    // TODO: add fields here
} Future;

static void future_initialize(Future* future, FutureThread future_thread, void* data)
{
    memset(future, 0, sizeof(*future));
    // TODO: initialize fields here
}

static void future_finalize(Future* future)
{
    // TODO: finalize fields here
}

Future* future_create(FutureThread future_thread, void* data)
{
    Future* future = malloc(sizeof(Future));
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
}

FutureStatus future_status(Future* future)
{
}

void* future_result(Future* future)
{
}

void future_notify_error(Future* future, void* result)
{
}
