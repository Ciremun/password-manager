// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#ifdef _MSC_VER
#pragma comment(lib, "gdi32")
#pragma comment(lib, "User32")
#endif // _MSC_VER

#ifndef __wasm__
#include "os_generic.h"
#endif // __wasm__

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"

#include "pm_util.h"

#define WINDOW_NAME "password-manager"

short w, h;
int paused = 0;

extern int offset;
extern char str[64];

void setup_window()
{
#ifdef __ANDROID__
    CNFGSetupFullscreen(WINDOW_NAME, 0);
    CNFGGetDimensions(&w, &h);
#elif defined(__wasm__)
    CNFGGetDimensions(&w, &h);
    CNFGSetup(WINDOW_NAME, w, h);
#else
    w = 900;
    h = 900;
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
#endif // RAWDRAW_USE_LOOP_FUNCTION
    {
        CNFGClearFrame();
        CNFGHandleInput();

#ifndef __wasm__
        if (!paused)
            OGUSleep(16000);
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
