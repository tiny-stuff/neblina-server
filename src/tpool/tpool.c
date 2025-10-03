#include "tpool.h"

static bool multithreaded;

void tpool_init(bool multithreaded_)
{
    multithreaded = multithreaded_;
}

void tpool_finalize()
{
}

void tpool_add_task(TPoolTask task, int idx, void* data)
{
    if (!multithreaded) {
        task(idx, data);
    }
}