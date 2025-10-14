#ifndef FUTURE_H_
#define FUTURE_H_
#include <stdbool.h>

typedef struct Future Future;
typedef void*(*FutureThread)(Future*, void*);
typedef enum { FU_SUCCESS, FU_ERROR, FU_RUNNING } FutureStatus;

Future*      future_create(FutureThread future_thread, void* data);
void         future_destroy(Future* future);

FutureStatus future_status(Future* future);

FutureStatus future_await(Future* future, void** result);

void         future_notify_error(Future* future, void* result);

#endif
