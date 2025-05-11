#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define global static
#define internal static

#if BUILD_DEBUG
#define ASSERT(cond)          \
    do {                      \
        if (!(cond)) {        \
            __builtin_trap(); \
        }                     \
    } while (0)
#define UNREACHABLE() ASSERT(false)
#else
#define ASSERT(cond) (void)(cond)
#define UNREACHABLE() __builtin_unreachable()
#endif

#define KB(n) ((n) << 10)
#define MB(n) ((n) << 20)

#define ARRAY_CAP(a) (sizeof(a) / sizeof((a)[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif //UTILS_H
