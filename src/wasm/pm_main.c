#ifndef __wasm__
#include "os_generic.h"
#endif // __wasm__

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

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

#define WINDOW_NAME "password-manager"

short w, h;
int offset = 0;
char str[64] = {0};

void EXPORT("HandleKey") HandleKey(int keycode, int bDown)
{
    if (bDown)
        str[offset++] = keycode;
}

void setup_window()
{
#ifdef __ANDROID__
    CNFGSetupFullscreen(WINDOW_NAME, 0);
    CNFGGetDimensions(&w, &h);
#elif defined(__wasm__)
    CNFGGetDimensions(&w, &h);
    CNFGSetup(WINDOW_NAME, w, h);
#else
    w = 1600;
    h = 800;
    CNFGSetup(WINDOW_NAME, w, h);
#endif // __ANDROID__
}

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

int EXPORT("main") main()
{
    CNFGBGColor = BLACK;
    setup_window();

#ifdef RAWDRAW_USE_LOOP_FUNCTION
    return 0;
}
int EXPORT("loop") loop()
{
#else
    while (1)
#endif
    {
        CNFGClearFrame();
        CNFGHandleInput();

#ifndef __wasm__
        if (!paused)
            OGUSleep(5000);
#endif // __wasm__

        CNFGColor(COLOR(0xffffffff));
        CNFGTackRectangle(w / 10, h / 2 + 25, w - w / 10, h / 2 - 25);

        CNFGPenX = w / 10 + 10;
        CNFGPenY = h / 2 - 10;
        CNFGColor(BLACK);
        CNFGDrawText(str, 5);
        
        CNFGSwapBuffers();
    }

    return 0;
}
