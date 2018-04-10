#ifndef CLOCK_H_
#define CLOCK_H_

#include "sized_types.h"

struct Clock
{
    s64 frequency;
};

bool set_up_clock(Clock* clock);
s64 get_timestamp(Clock* clock);
s64 get_millisecond_duration(Clock* clock, s64 start, s64 end);
s64 get_nanosecond_duration(Clock* clock, s64 start, s64 end);
s64 start_timing(Clock* clock);
s64 stop_timing(Clock* clock, s64 start);

void escape(void* p);

#endif // CLOCK_H_
