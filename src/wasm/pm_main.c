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

        CNFGSwapBuffers();
    }

    return 0;
}
