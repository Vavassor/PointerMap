#ifndef MEMORY_H_
#define MEMORY_H_

#include "sized_types.h"

#include <cstdlib>

// Note from Andrew Dawson: This is all part of an interface I normally use for
// memory. It's not relevant to these tests, so here they're replaced with stubs
// that just use calloc and free.

struct Heap
{
    int placeholder;
};

#define heap_create(heap, bytes)
#define heap_destroy(heap)

#define HEAP_ALLOCATE(heap, type, count)\
    static_cast<type*>(calloc(count, sizeof(type)))

#define HEAP_DEALLOCATE(heap, array)\
    free(array)

#define SAFE_HEAP_DEALLOCATE(heap, array)\
    {free(array); (array) = nullptr;}

#endif // MEMORY_H_
