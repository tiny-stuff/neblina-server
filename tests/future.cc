#include "doctest.h"

#include <stdlib.h>

extern "C" {
#include "util/future.h"
#include "os.h"
}

static void* task_success_after_50ms(Future* future, void*)
{
    os_sleep_ms(50);
    return (void *) EXIT_SUCCESS;
}

static void* task_immediate_failure(Future* future, void*)
{
    future_notify_error(future, (void *) EXIT_FAILURE);
    return NULL;
}

TEST_SUITE("Futures")
{
    TEST_CASE("Futures")
    {
        SUBCASE("Failure")
        {
            Future* future = future_create(task_immediate_failure, NULL);
            void* result;
            CHECK(future_await(future, &result) == FU_ERROR);
            CHECK(result == (void *) EXIT_FAILURE);
            future_destroy(future);
        }

        SUBCASE("Success")
        {
            Future* future = future_create(task_success_after_50ms, NULL);
            CHECK(future_status(future) == FU_RUNNING);
            void* result;
            CHECK(future_await(future, &result) == FU_SUCCESS);
            CHECK(result == (void *) EXIT_SUCCESS);
            future_destroy(future);
        }
    }
}