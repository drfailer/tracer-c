#include "tracer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/******************************************************************************/
/*                                   tracer                                   */
/******************************************************************************/


TracerHandle *tracer_create(size_t nb_global_regions)
{
    TracerHandle *tracer = malloc(sizeof(*tracer));
    memset(tracer, 0, sizeof(*tracer));
    tracer->data = tracer_data_create();
    tracer->start = tracer_tp_get();
    if (nb_global_regions > 0) {
        array_create(tracer->global_regions, nb_global_regions);
    }
    return tracer;
}

void tracer_destroy(TracerHandle *tracer)
{
    tracer_data_destroy(&tracer->data);
    array_destroy(tracer->global_regions);
    free(tracer);
}

TracerTimePoint tracer_tp_get()
{
    TracerTimePoint tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp;
}

// TODO: add configurable unit
TracerDuration tracer_tp_dur(TracerTimePoint t_start, TracerTimePoint t_end)
{
    return (1000.0 * t_end.tv_sec + 1e-6 * t_end.tv_nsec) - (1000.0 * t_start.tv_sec + 1e-6 * t_start.tv_nsec);
}

TracerTimestamp tracer_timestamp_from_timepoint(TracerTimePoint tp)
{
    return (long long)tp.tv_sec * 1000000000LL + tp.tv_nsec;
}

TracerTimePoint tracer_tp_diff(TracerTimePoint begin, TracerTimePoint end) {
    end.tv_sec -= begin.tv_sec;
    end.tv_nsec -= begin.tv_nsec;
    return end;
}

void tracer_add_ev(TracerHandle *tracer, char *group, char *timeline)
{
    TracerTimePoint time_point = tracer_tp_diff(tracer->start, tracer_tp_get());
    TracerTimestamp ts = tracer_timestamp_from_timepoint(time_point);
    tracer_data_add_trace(&tracer->data, ts, ts, group, timeline);
}

void tracer_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline)
{
    TracerTimePoint begin_tp = tracer_tp_diff(tracer->start, begin);
    TracerTimePoint end_tp = tracer_tp_diff(tracer->start, end);
    tracer_data_add_trace(&tracer->data, tracer_timestamp_from_timepoint(begin_tp),
                          tracer_timestamp_from_timepoint(end_tp), group, timeline);
}

/******************************************************************************/
/*                                tracer data                                 */
/******************************************************************************/

TracerData tracer_data_create()
{
    TracerData data = {0};
    array_create(data.traces, 64);
    return data;
}

void tracer_data_destroy(TracerData *data)
{
    for (size_t t = 0; t < data->traces.len; ++t) {
        free(data->traces.ptr[t].group);
        free(data->traces.ptr[t].timeline);
    }
    array_destroy(data->traces);
}

void tracer_data_add_trace(TracerData *data, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline)
{
    Trace trace = {
        .begin = begin,
        .end = end,
        .group = strdup(group),
        .timeline = strdup(timeline),
    };
    array_append(data->traces, trace);
}

void tracer_data_write(TracerData const *data, char const *filename)
{
    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        fprintf(stderr, "error: cannot open file '%s'.\n", filename);
        return;
    }

    for (size_t t = 0; t < data->traces.len; ++t) {
        Trace *trace = &data->traces.ptr[t];
        if (trace->begin == trace->end) {
            fprintf(file, "ev;%lld;%s;%s\n", trace->begin, trace->group, trace->timeline);
        } else {
            fprintf(file, "dur;%lld,%lld;%s;%s\n", trace->begin, trace->end, trace->group, trace->timeline);
        }
    }

    fclose(file);
}

/******************************************************************************/
/*                                  regions                                   */
/******************************************************************************/

TracerLocalRegion tracer_local_region_begin(TracerHandle *tracer, char *group, char *timeline)
{
    return (TracerLocalRegion){
        .done = false,
        .tracer = tracer,
        .begin = tracer_tp_get(),
        .group = group,
        .timeline = timeline,
    };
}

void tracer_local_region_end(TracerLocalRegion *tr)
{
    tracer_add_dur(tr->tracer, tr->begin, tracer_tp_get(), tr->group, tr->timeline);
    tr->done = true;
}

void tracer_region_begin(TracerHandle *tracer, size_t idx, char *group, char *timeline)
{
    if (idx > tracer->global_regions.len) {
        fprintf(stderr, "error: global region index `%ld` too high.\n", idx);
        return;
    }
    tracer->global_regions.ptr[idx] = (TracerRegion){
        .group = group,
        .timeline = timeline,
        .begin = tracer_tp_get(),
    };
}

void tracer_region_end(TracerHandle *tracer, size_t idx, char *group, char *timeline)
{
    TracerTimePoint end = tracer_tp_get();

    if (idx > tracer->global_regions.len) {
        fprintf(stderr, "error: global region index `%ld` too high.\n", idx);
        return;
    }
    assert(strcmp(tracer->global_regions.ptr[idx].group, group) == 0);
    assert(strcmp(tracer->global_regions.ptr[idx].timeline, timeline) == 0);
    tracer_add_dur(tracer, tracer->global_regions.ptr[idx].begin, end, group, timeline);
}
