#ifndef RANDOM_H_
#define RANDOM_H_

#include "sized_types.h"

struct Sequence
{
    u64 s[2];
    u64 seed;
};

u64 generate(Sequence* sequence);
u64 seed(Sequence* sequence, u64 value);
int random_int_range(Sequence* sequence, int min, int max);

#endif // RANDOM_H_
