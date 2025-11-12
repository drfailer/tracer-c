#include <stdio.h>
#include <time.h>
#include <unistd.h>
#define ENABLE_TRACER
#include "tracer.h"

int main(void) {
    TracerHandle *tracer = TRACER_CREATE("test.tr", 1);

    TRACER_REGION_BEGIN(tracer, 0, "global region", "t1");
    TRACER_LOCAL_REGION(tracer, "local region", "t2")
    {
        TRACER_ADD_EV(tracer, "group2", "t1");
        TRACER_REGION_END(tracer, 0, "global region", "t1");
        TRACER_TIMER_START(sleep);
        sleep(1);
        TRACER_TIMER_END(sleep);
        TRACER_ADD_DUR(tracer, begin_sleep, end_sleep, "sleep", "t3");
    }

    TRACER_REGION_BEGIN(tracer, 0, "global region with infos", "t1");
    TRACER_LOCAL_REGION(tracer, "local region with infos", "t2", "test %d", 42)
    {
        TRACER_ADD_EV(tracer, "group2 with infos", "t1", "info");
        TRACER_REGION_END(tracer, 0, "global region with infos", "t1", "info");
        TRACER_TIMER_START(sleep);
        sleep(1);
        TRACER_TIMER_END(sleep);
        TRACER_ADD_DUR(tracer, begin_sleep, end_sleep, "sleep with info", "t3", "info");
    }

    TRACER_DESTROY(tracer);
    return 0;
}
