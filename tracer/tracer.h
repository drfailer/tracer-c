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
/*                                   macros                                   */
/******************************************************************************/

#define tracer_timer_start(timer_name) TracerTimePoint begin_##timer_name = tracer_tp_get();
#define tracer_timer_end(timer_name) TracerTimePoint end_##timer_name = tracer_tp_get();
#define tracer_timer_dur(timer_name) tracer_tp_dur(begin_##timer_name, end_##timer_name);

typedef struct {
    bool done;
    TracerHandle *tracer;
    TracerTimePoint begin;
    char *group;
    char *timeline;
} TracerLocalRegion;

TracerLocalRegion tracer_local_region_begin(TracerHandle *tracer, char *group, char *timeline);
void tracer_local_region_end(TracerLocalRegion *tr);

#define tracer_local_region(tracer, group, timeline) \
    for (TracerLocalRegion tr = tracer_local_region_begin(tracer, group, timeline); \
         !tr.done; \
         tracer_local_region_end(&tr))

void tracer_region_begin(TracerHandle *tracer, size_t idx, char *group, char *timeline);
void tracer_region_end(TracerHandle *tracer, size_t idx, char *group, char *timeline);

#endif
