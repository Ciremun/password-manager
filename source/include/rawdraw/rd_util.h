#ifndef PM_RD_UTIL_H_
#define PM_RD_UTIL_H_

#include <stddef.h>
#include <stdint.h>

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif // min

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif // min

#ifdef __wasm__
#define EXPORT(s) __attribute__((export_name(s)))
#define SWAPS(v) ((v >> 24) & 0xff) | ((v << 8) & 0xff0000) | ((v >> 8) & 0xff00) | ((v << 24) & 0xff000000)
#define COLOR(c) SWAPS(c)
#else
#define COLOR(c) c
#define EXPORT(s)
#endif // __wasm__

#define BLACK COLOR(0X000000FF)
#define WHITE COLOR(0XFFFFFFFF)

#define SPACE_KEY 32
#define ZERO_KEY 48
#define ONE_KEY 49
#define TWO_KEY 50
#define NINE_KEY 57

#ifdef __wasm__
#define R_KEY 82
#define LMB_KEY 0
#elif defined(__ANDROID__)
#define LMB_KEY 0
#define R_KEY 114
#else
#define LMB_KEY 1
#define R_KEY 114
#endif // __wasm__

#if defined(_WIN32) || defined(__wasm__)
#define MINUS_KEY 189
#define PLUS_KEY 187
#define RMB_KEY 2
#define BACKSPACE_KEY 8
#else
#define MINUS_KEY 45
#define PLUS_KEY 43
#define EQ_KEY 61
#define RMB_KEY 3
#define BACKSPACE_KEY 8
#endif // defined(_WIN32) || defined(__wasm__)

#ifdef __wasm__
#define printf
#define free
unsigned long strlen(const char *s);
void *memset(void *dest, int val, unsigned long len);
void *memcpy(void *dst, void const *src, unsigned long size);
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void print(double idebug);
void prints(const char *str);
#else
#define print
#define prints
#endif // __wasm__

typedef struct
{
    int x;
    int y;
} Point;

#ifdef __wasm__
typedef struct
{
    uint8_t *data;
    size_t length;
} String;
#endif // __wasm__

#endif // PM_RD_UTIL_H_