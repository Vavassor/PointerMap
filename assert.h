#ifndef ASSERT_H_
#define ASSERT_H_

#include <cassert>

// Note from Andrew Dawson: I use my own assert functions normally, but for
// brevity just redirect to standard C assert.

#if defined(NDEBUG)
#define ASSERT(expression) static_cast<void>(0)
#else
#define ASSERT(expression) assert(expression)
#endif

#endif // ASSERT_H_
