#include "doctest.h"

extern "C" {
#include "os/os.h"
#include "os/timer.h"
}

TEST_SUITE("Timer")
{
    TEST_CASE("Timer")
    {
        Timer* timer = timer_create_();
        os_sleep_ms(10);
        uint64_t t = timer_current_ms(timer);
        CHECK(t >= 9);
        CHECK(t < 100);
    }
}