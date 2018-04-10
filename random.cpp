#include "random.h"

static uint64_t splitmix64(uint64_t* x)
{
    *x += UINT64_C(0x9E3779B97F4A7C15);
    uint64_t z = *x;
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

static u64 rotl(const u64 x, int k)
{
    return (x << k) | (x >> (64 - k));
}

// Xoroshiro128+
u64 generate(Sequence* sequence)
{
    const u64 s0 = sequence->s[0];
    u64 s1 = sequence->s[1];
    const u64 result = s0 + s1;

    s1 ^= s0;
    sequence->s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    sequence->s[1] = rotl(s1, 36); // c

    return result;
}

u64 seed(Sequence* sequence, u64 value)
{
    u64 old_seed = sequence->seed;
    sequence->seed = value;
    sequence->s[0] = splitmix64(&sequence->seed);
    sequence->s[1] = splitmix64(&sequence->seed);
    return old_seed;
}

int random_int_range(Sequence* sequence, int min, int max)
{
    int x = generate(sequence) % static_cast<u64>(max - min + 1);
    return min + x;
}
