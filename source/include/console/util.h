#ifndef PM_UTIL_H_
#define PM_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

typedef struct
{
    uint8_t *data;
    size_t length;
} String;

#define PM_STR(s) \
    (String) { .data = (uint8_t *)s, .length = strlen(s) }

#ifdef TEST
void exit_test_case(int exit_code);
#define exit exit_test_case
#define error(fmt, ...)
#define info(fmt, ...)
#elif defined(__ANDROID__)
#include <android/log.h>
#define error(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "ImGuiExample", "error: " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "ImGuiExample", "info: " fmt "\n", __VA_ARGS__)
#else
#define error(fmt, ...) fprintf(stderr, "error: " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) fprintf(stdout, "info: " fmt "\n", __VA_ARGS__)
#endif // TEST

#define PANIC(FMT, ...)          \
    do                           \
    {                            \
        error(FMT, __VA_ARGS__); \
        exit(1);                 \
    } while (0)

#define ASSERT_ALLOC(ptr)                                            \
    do                                                               \
    {                                                                \
        if (ptr == 0)                                                \
        {                                                            \
            error("memory allocation failed (%s)", strerror(errno)); \
            exit(1);                                                 \
        }                                                            \
    } while (0)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#endif // PM_UTIL_H_
