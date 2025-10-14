#include "future.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Future {
    // TODO: add fields here
} Future;

static void future_initialize(Future* future)
{
    memset(future, 0, sizeof(*future));
    // TODO: initialize fields here
}

static void future_finalize(Future* future)
{
    // TODO: finalize fields here
}

Future* future_create()
{
    Future* future = malloc(sizeof(Future));
    if (future == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    future_initialize(future);
    return future;
}

void future_destroy(Future* future)
{
    future_finalize(future);
    free(future);
}
