#ifndef PM_UTIL_H_
#define PM_UTIL_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t *data;
    size_t length;
} String;

#define PANIC(FMT, ...)          \
    do                           \
    {                            \
        error(FMT, __VA_ARGS__); \
        exit(1);                 \
    } while (0)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#endif // PM_UTIL_H_
