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

#include <stddef.h>

#include "pm_util.h"

#define WINDOW_NAME "password-manager"

typedef struct
{
    char *data;
    size_t length;
} String;

typedef struct
{
    String string;
    uint32_t color;
    int font_size;
} Text;

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
    Point p1;
    Point p2;
    uint32_t color;
} Rect;

typedef struct
{
    Rect rect;
    Text text;
} InputField;

short w, h;
int paused = 0;

extern int offset;
extern char str[64];

void DrawInputField(InputField i)
{
    static const int text_height = 10;
    int rect_height = i.rect.p2.y - i.rect.p1.y;
    CNFGPenX = i.rect.p1.x + 10;
    CNFGPenY = i.rect.p1.y + rect_height / 2 - text_height;
    CNFGColor(i.rect.color);
    CNFGTackRectangle(i.rect.p1.x, i.rect.p1.y, i.rect.p2.x, i.rect.p2.y);
    CNFGColor(i.text.color);
    CNFGDrawText(i.text.string.data, i.text.font_size);
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
    w = 900;
    h = 900;
    CNFGSetup(WINDOW_NAME, w, h);
#endif // __ANDROID__
}

int EXPORT("main") main()
{
    CNFGBGColor = BLACK;
    setup_window();

    InputField i = {
        .rect = (Rect){
            .color = WHITE,
            .p1 = (Point){
                .x = 0,
                .y = 0,
            },
            .p2 = (Point){
                .x = w,
                .y = 40,
            },
        },
        .text = (Text){
            .string = (String){
                .data = str,
                .length = 0,
            },
            .color = BLACK,
            .font_size = 5,
        },
    };

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

        DrawInputField(i);

        CNFGSwapBuffers();
    }

    return 0;
}
