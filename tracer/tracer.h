#ifndef TRACER
#define TRACER
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "array.h"

/*
 * Size of the temporary buffer used to create the info string
 */
#ifndef MAX_INFO_STR_SIZE
#define MAX_INFO_STR_SIZE 1024
#endif

/******************************************************************************/
/*                                   types                                    */
/******************************************************************************/

typedef struct timespec TracerTimePoint;
typedef long long TracerTimestamp;
typedef double TracerDuration;

typedef struct {
    TracerTimePoint begin;
    char *group;
    char *timeline;
} TracerRegion;

Array(TracerRegion) TracerRegions;

typedef struct {
    TracerTimePoint start;
    TracerRegions global_regions;
    bool enabled;
    FILE *file;
} TracerHandle;

/******************************************************************************/
/*                                   tracer                                   */
/******************************************************************************/

TracerHandle *tracer_create(char *filename, size_t nb_global_regions);
void tracer_destroy(TracerHandle *tracer);

TracerTimePoint tracer_tp_get();
TracerDuration tracer_tp_dur(TracerTimePoint t_start, TracerTimePoint t_end);

void tracer_v_add_trace(TracerHandle *handle, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline, char *infos, va_list list);
void tracer_add_trace(TracerHandle *handle, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline);
void tracer_add_trace_with_info(TracerHandle *handle, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline, char *infos, ...);

void tracer_add_ev(TracerHandle *tracer, char *group, char *timeline);
void tracer_add_ev_with_infos(TracerHandle *tracer, char *group, char *timeline, char *infos, ...);

void tracer_v_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline, char *infos, va_list list);
void tracer_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline);
void tracer_add_dur_with_infos(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline, char *infos, ...);

/******************************************************************************/
/*                                  regions                                   */
/******************************************************************************/

typedef struct {
    bool done;
    TracerHandle *tracer;
    TracerTimePoint begin;
    char *group;
    char *timeline;
} TracerLocalRegion;

TracerLocalRegion tracer_local_region_begin(TracerHandle *tracer, char *group, char *timeline);
void tracer_local_region_end(TracerLocalRegion *tr, bool has_infos, char *infos, ...);

void tracer_region_begin(TracerHandle *tracer, size_t idx, char *group, char *timeline);
void tracer_region_end(TracerHandle *tracer, size_t idx, char *group, char *timeline);
void tracer_region_end_with_infos(TracerHandle *tracer, size_t idx, char *group, char *timeline, char *infos, ...);

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

#ifdef ENABLE_TRACER

#define TRACER_TIMER_START(timer_name) TracerTimePoint begin_##timer_name = tracer_tp_get();
#define TRACER_TIMER_END(timer_name) TracerTimePoint end_##timer_name = tracer_tp_get();
#define TRACER_TIMER_DUR(timer_name) tracer_tp_dur(begin_##timer_name, end_##timer_name);

#define TRACER_ADD_EV(tracer, group, timeline, ...)                            \
    do {                                                                       \
        if (sizeof(#__VA_ARGS__) > 1) {                                        \
            tracer_add_ev_with_infos(tracer, group, timeline, "" __VA_ARGS__); \
        } else {                                                               \
            tracer_add_ev(tracer, group, timeline);                            \
        }                                                                      \
    } while (false);
#define TRACER_ADD_DUR(tracer, tbegin, tend, group, timeline, ...)                            \
    do {                                                                                      \
        if (sizeof(#__VA_ARGS__) > 1) {                                                       \
            tracer_add_dur_with_infos(tracer, tbegin, tend, group, timeline, "" __VA_ARGS__); \
        } else {                                                                              \
            tracer_add_dur(tracer, tbegin, tend, group, timeline);                            \
        }                                                                                     \
    } while (false);

#define TRACER_REGION_BEGIN(tracer, idx, group, timeline) tracer_region_begin(tracer, idx, group, timeline)
#define TRACER_REGION_END(tracer, idx, group, timeline, ...)                                  \
    do {                                                                                      \
        if (sizeof(#__VA_ARGS__) > 0) {                                                       \
            tracer_region_end_with_infos(tracer, idx, group, timeline, "" __VA_ARGS__);       \
        } else {                                                                              \
            tracer_region_end(tracer, idx, group, timeline);                                  \
        }                                                                                     \
    } while (false);

#define TRACER_LOCAL_REGION(tracer, group, timeline, ...)                           \
    for (TracerLocalRegion tr = tracer_local_region_begin(tracer, group, timeline); \
         !tr.done;                                                                  \
         tracer_local_region_end(&tr, sizeof(#__VA_ARGS__) > 1, "" __VA_ARGS__))

#define TRACER_CREATE(filename, nb_global_regions) tracer_create(filename, nb_global_regions)
#define TRACER_DESTROY(tracer) tracer_destroy(tracer)

#else // ENABLE_TRACER

#define TRACER_TIMER_START(timer_name)
#define TRACER_TIMER_END(timer_name)
#define TRACER_TIMER_DUR(timer_name)

#define TRACER_ADD_EV(tracer, group, timeline, ...)
#define TRACER_ADD_DUR(tracer, tbegin, tend, group, timeline, ...)

#define TRACER_REGION_BEGIN(tracer, idx, group, timeline)
#define TRACER_REGION_END(tracer, idx, group, timeline, ...)

#define TRACER_LOCAL_REGION(tracer, group, timeline, ...)

#define TRACER_CREATE(filename, nb_global_regions) NULL
#define TRACER_DESTROY(tracer)

#endif // ENABLE_TRACER

#endif
