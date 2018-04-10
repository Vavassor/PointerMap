#include "clock.h"

#include "platform_definitions.h"

#if defined(OS_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <intrin.h>
#else
#include <ctime>
#endif

namespace
{
    const s64 milliseconds_per_second = 1000;
    const s64 nanoseconds_per_second = 1000000000;
    const s64 nanoseconds_per_millisecond = 1000000;
}

#if defined(OS_WINDOWS)

bool set_up_clock(Clock* clock)
{
    LARGE_INTEGER frequency;
    bool got = QueryPerformanceFrequency(&frequency) != 0;
    if(got)
    {
        clock->frequency = frequency.QuadPart;
    }
    return got;
}

s64 get_timestamp(Clock* clock)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart;
}

s64 get_millisecond_duration(Clock* clock, s64 start, s64 end)
{
    return ((end - start) * milliseconds_per_second) / clock->frequency;
}

s64 get_nanosecond_duration(Clock* clock, s64 start, s64 end)
{
    return ((end - start) * nanoseconds_per_second) / clock->frequency;
}

static void clobber()
{
    _ReadWriteBarrier();
}

#pragma optimize("", off)
void escape(void* p)
{
    void* l = p;
}
#pragma optimize("", on)

#else

bool set_up_clock(Clock* clock)
{
    return true;
}

s64 get_timestamp(Clock* clock)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (nanoseconds_per_second * now.tv_sec) + now.tv_nsec;
}

s64 get_millisecond_duration(Clock* clock, s64 start, s64 end)
{
    return (end - start) / nanoseconds_per_millisecond;
}

s64 get_nanosecond_duration(Clock* clock, s64 start, s64 end)
{
    return end - start;
}

void escape(void* p)
{
    asm volatile("" : : "g"(p) : "memory");
}

static void clobber()
{
    asm volatile("" : : : "memory");
}

#endif // defined(OS_WINDOWS)

s64 start_timing(Clock* clock)
{
    s64 start = get_timestamp(clock);
    clobber();
    return start;
}

s64 stop_timing(Clock* clock, s64 start)
{
    clobber();
    s64 end = get_timestamp(clock);
    s64 milliseconds = get_millisecond_duration(clock, start, end);
    return milliseconds;
}
