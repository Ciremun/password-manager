#ifndef PM_UTIL_H__
#define PM_UTIL_H__

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif // min

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif // min

#ifdef __wasm__
#define EXPORT(s) __attribute__((export_name(s)))
#define SWAPS(v) ((v>>24)&0xff) | ((v<<8)&0xff0000) | ((v>>8)&0xff00) | ((v<<24)&0xff000000)
#define COLOR(c) SWAPS(c)
#else
#define COLOR(c) c
#define EXPORT(s)
#endif // __wasm__

#define BLACK COLOR(0X000000FF)

#ifdef __wasm__
unsigned long strlen(const char *s);
void *memset(void *dest, int val, unsigned long len);
void *memcpy(void *dst, void const *src, unsigned long size);
#endif // __wasm__

void print(double idebug);
void prints(const char *str);

#endif // PM_UTIL_H__
