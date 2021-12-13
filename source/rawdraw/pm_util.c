// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pm_util.h"

int inside_rect(Point p, Rect r)
{
    return p.x >= r.p1.x && p.y >= r.p1.y && p.x <= r.p2.x && p.y <= r.p2.y;
}

#ifdef __wasm__

extern unsigned char __heap_base;
char *heap = (char *)&__heap_base;

unsigned long strlen(const char *s)
{
    unsigned long sz = 0;
    while (s[sz] != '\0')
        sz++;
    return sz;
}

void *memset(void *dest, int val, unsigned long len)
{
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void *memcpy(void *dst, void const *src, unsigned long size)
{
    unsigned char *source = (unsigned char *)src;
    unsigned char *dest = (unsigned char *)dst;
    while (size--)
        *dest++ = *source++;
    return dst;
}

#endif // __wasm__
