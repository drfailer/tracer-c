#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "tracer.h"

int main(void) {
    TracerHandle *tracer = tracer_create(1);
    tracer_region_begin(tracer, 0, "global region", "t1");
    tracer_local_region(tracer, "local region", "t1")
    {
        tracer_timer_start(sleep);
        sleep(1);
        tracer_timer_end(sleep);
        tracer_add_dur(tracer, begin_sleep, end_sleep, "sleep", "t1");
        tracer_add_ev(tracer, "group2", "t1");
    }
    tracer_region_end(tracer, 0, "global region", "t1");
    tracer_data_write(&tracer->data, "test.tr");
    tracer_destroy(tracer);
    return 0;
}
