// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "rawdraw/rd_util.h"

Keyboard keyboard = {0};

#ifdef __wasm__

extern unsigned char __heap_base;
char *heap = (char *)&__heap_base;

size_t strlen(const char *s)
{
    size_t sz = 0;
    while (s[sz] != '\0')
        sz++;
    return sz;
}

void *memset(void *dest, int val, size_t len)
{
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void *memcpy(void *dst, void const *src, size_t size)
{
    unsigned char *source = (unsigned char *)src;
    unsigned char *dest = (unsigned char *)dst;
    while (size--)
        *dest++ = *source++;
    return dst;
}

void *malloc(size_t size)
{
    heap += size;
    return heap - size;
}

void *calloc(size_t num, size_t size)
{
    return malloc(num * size);
}

#endif // __wasm__
