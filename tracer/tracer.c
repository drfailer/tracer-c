#include "tracer.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/******************************************************************************/
/*                                   tracer                                   */
/******************************************************************************/


TracerHandle *tracer_create(char *filename, size_t nb_global_regions)
{
    TracerHandle *tracer = malloc(sizeof(*tracer));
    memset(tracer, 0, sizeof(*tracer));

    tracer->file = fopen(filename, "w+");
    assert(tracer->file != NULL);
    if (nb_global_regions > 0) {
        array_create(tracer->global_regions, nb_global_regions);
    }
    tracer->start = tracer_tp_get();
    tracer->enabled = true;
    return tracer;
}

void tracer_destroy(TracerHandle *tracer)
{
    fclose(tracer->file);
    array_destroy(tracer->global_regions);
    free(tracer);
}

void tracer_v_add_trace(TracerHandle *tracer, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline, char *infos, va_list list)
{
    char info_str[MAX_INFO_STR_SIZE] = {0};

    if (!tracer->enabled) {
        return;
    }

    vsnprintf(info_str, MAX_INFO_STR_SIZE, infos, list);
    if (begin == end) {
        fprintf(tracer->file, "ev;%lld;%s;%s;%s\n", begin, group, timeline, info_str);
    } else {
        fprintf(tracer->file, "dur;%lld,%lld;%s;%s;%s\n", begin, end, group, timeline, info_str);
    }
}

void tracer_add_trace(TracerHandle *tracer, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline)
{
    if (!tracer->enabled) {
        return;
    }

    if (begin == end) {
        fprintf(tracer->file, "ev;%lld;%s;%s\n", begin, group, timeline);
    } else {
        fprintf(tracer->file, "dur;%lld,%lld;%s;%s\n", begin, end, group, timeline);
    }
}

void tracer_add_trace_with_info(TracerHandle *tracer, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline, char *infos, ...)
{
    va_list list;
    va_start(list, infos);
    tracer_v_add_trace(tracer, begin, end, group, timeline, infos, list);
    va_end(list);
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
    tracer_add_trace(tracer, ts, ts, group, timeline);
}

void tracer_add_ev_with_infos(TracerHandle *tracer, char *group, char *timeline, char *infos, ...)
{
    va_list list;
    TracerTimePoint time_point = tracer_tp_diff(tracer->start, tracer_tp_get());
    TracerTimestamp ts = tracer_timestamp_from_timepoint(time_point);

    va_start(list, infos);
    tracer_v_add_trace(tracer, ts, ts, group, timeline, infos, list);
    va_end(list);
}

void tracer_v_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline, char *infos, va_list list)
{
    TracerTimePoint begin_tp = tracer_tp_diff(tracer->start, begin);
    TracerTimePoint end_tp = tracer_tp_diff(tracer->start, end);
    tracer_v_add_trace(tracer, tracer_timestamp_from_timepoint(begin_tp),
                     tracer_timestamp_from_timepoint(end_tp), group, timeline, infos, list);
}

void tracer_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline)
{
    TracerTimePoint begin_tp = tracer_tp_diff(tracer->start, begin);
    TracerTimePoint end_tp = tracer_tp_diff(tracer->start, end);
    tracer_add_trace(tracer, tracer_timestamp_from_timepoint(begin_tp),
                     tracer_timestamp_from_timepoint(end_tp), group, timeline);
}

void tracer_add_dur_with_infos(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline, char *infos, ...)
{
    va_list list;
    va_start(list, infos);
    tracer_v_add_dur(tracer, begin, end, group, timeline, infos, list);
    va_end(list);
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

void tracer_local_region_end(TracerLocalRegion *tr, bool has_infos, char *infos, ...)
{
    if (has_infos) {
        va_list list;
        va_start(list, infos);
        tracer_v_add_dur(tr->tracer, tr->begin, tracer_tp_get(), tr->group, tr->timeline, infos, list);
        va_end(list);
    } else {
        tracer_add_dur(tr->tracer, tr->begin, tracer_tp_get(), tr->group, tr->timeline);
    }
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

void tracer_region_end_with_infos(TracerHandle *tracer, size_t idx, char *group, char *timeline, char *infos, ...)
{
    va_list list;
    TracerTimePoint end = tracer_tp_get();

    if (idx > tracer->global_regions.len) {
        fprintf(stderr, "error: global region index `%ld` too high.\n", idx);
        return;
    }
    assert(strcmp(tracer->global_regions.ptr[idx].group, group) == 0);
    assert(strcmp(tracer->global_regions.ptr[idx].timeline, timeline) == 0);
    va_start(list, infos);
    tracer_v_add_dur(tracer, tracer->global_regions.ptr[idx].begin, end, group, timeline, infos, list);
    va_end(list);
}
