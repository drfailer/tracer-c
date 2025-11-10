#ifndef TRACER
#define TRACER
#include <time.h>
#include <stdbool.h>
#include "array.h"

/******************************************************************************/
/*                                   types                                    */
/******************************************************************************/

typedef struct timespec TracerTimePoint;
typedef long long TracerTimestamp;
typedef double TracerDuration;

typedef struct {
    TracerTimestamp begin;
    TracerTimestamp end;
    char *group;
    char *timeline;
} Trace;

Array(Trace) Traces;

typedef struct {
    Traces traces;
} TracerData;

typedef struct {
    char *group;
    char *timeline;
    TracerTimePoint begin;
} TracerRegion;

Array(TracerRegion) TracerRegions;

typedef struct {
    TracerData data;
    TracerTimePoint start;
    TracerRegions global_regions;
} TracerHandle;

/******************************************************************************/
/*                                   tracer                                   */
/******************************************************************************/

TracerHandle *tracer_create(size_t nb_global_regions);
void tracer_destroy(TracerHandle *tracer);

TracerTimePoint tracer_tp_get();
TracerDuration tracer_tp_dur(TracerTimePoint t_start, TracerTimePoint t_end);
void tracer_add_ev(TracerHandle *tracer, char *group, char *timeline);
void tracer_add_dur(TracerHandle *tracer, TracerTimePoint begin, TracerTimePoint end, char *group, char *timeline);

/******************************************************************************/
/*                                tracer data                                 */
/******************************************************************************/

TracerData tracer_data_create();
void tracer_data_destroy(TracerData *data);
void tracer_data_add_trace(TracerData *data, TracerTimestamp begin, TracerTimestamp end, char *group, char *timeline);
void tracer_data_write(TracerData const *data, char const *filename);

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
void tracer_local_region_end(TracerLocalRegion *tr);

void tracer_region_begin(TracerHandle *tracer, size_t idx, char *group, char *timeline);
void tracer_region_end(TracerHandle *tracer, size_t idx, char *group, char *timeline);

/******************************************************************************/
/*                                   macros                                   */
/******************************************************************************/

#ifdef ENABLE_TRACER

#define TRACER_TIMER_START(timer_name) TracerTimePoint begin_##timer_name = tracer_tp_get();
#define TRACER_TIMER_END(timer_name) TracerTimePoint end_##timer_name = tracer_tp_get();
#define TRACER_TIMER_DUR(timer_name) tracer_tp_dur(begin_##timer_name, end_##timer_name);

#define TRACER_ADD_EV(tracer, group, timeline) tracer_add_ev(tracer, group, timeline)
#define TRACER_ADD_DUR(tracer, tbegin, tend, group, timeline) tracer_add_dur(tracer, tbegin, tend, group, timeline)

#define TRACER_REGION_BEGIN(tracer, idx, group, timeline) tracer_region_begin(tracer, idx, group, timeline)
#define TRACER_REGION_END(tracer, idx, group, timeline) tracer_region_end(tracer, idx, group, timeline)

#define TRACER_LOCAL_REGION(tracer, group, timeline) \
    for (TracerLocalRegion tr = tracer_local_region_begin(tracer, group, timeline); \
         !tr.done; \
         tracer_local_region_end(&tr))

#define TRACER_CREATE(nb_global_regions) tracer_create(nb_global_regions)
#define TRACER_DESTROY(tracer) tracer_destroy(tracer)
#define TRACER_WRITE(tracer, filename) tracer_data_write(&tracer->data, filename)

#else // ENABLE_TRACER

#define TRACER_TIMER_START(timer_name)
#define TRACER_TIMER_END(timer_name)
#define TRACER_TIMER_DUR(timer_name)

#define TRACER_ADD_EV(tracer, group, timeline)
#define TRACER_ADD_DUR(tracer, tbegin, tend, group, timeline)

#define TRACER_REGION_BEGIN(tracer, idx, group, timeline)
#define TRACER_REGION_END(tracer, idx, group, timeline)

#define TRACER_LOCAL_REGION(tracer, group, timeline)

#define TRACER_CREATE(nb_global_regions) NULL
#define TRACER_DESTROY(tracer)
#define TRACER_WRITE(tracer, filename)

#endif // ENABLE_TRACER

#endif
