#ifndef PM_UTIL_H__
#define PM_UTIL_H__

#ifdef __wasm__
#define EXPORT(s) __attribute__((export_name(s)))
#else
#define EXPORT(s)
#endif // __wasm__

#ifdef __wasm__
#define SWAPS(v) ((v>>24)&0xff) | ((v<<8)&0xff0000) | ((v>>8)&0xff00) | ((v<<24)&0xff000000)
#define COLOR(c) SWAPS(c)
#else
#define COLOR(c) c
#endif // __wasm__

#define BLACK COLOR(0X000000FF)

#ifdef __wasm__
unsigned long strlen(const char *s);
void *memset(void *dest, int val, unsigned long len);
void *memcpy(void *dst, void const *src, unsigned long size);
#endif // __wasm__

#endif // PM_UTIL_H__
