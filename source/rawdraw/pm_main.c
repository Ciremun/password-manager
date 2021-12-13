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
InputFields input_fields;

void add_input_field(InputField i)
{
    input_fields.arr[input_fields.count++] = i;
}

void oninput_i(InputField *i, int keycode)
{
    if (!i->focused)
        return;
    if (keycode == BACKSPACE_KEY)
    {
        if (i->text.offset)
            i->text.string.data[--i->text.offset] = 0;
    }
    else
        i->text.string.data[i->text.offset++] = keycode;
}

void DrawInputField(InputField i)
{
    static const int text_height = 10;
    int rect_height = i.rect.p2.y - i.rect.p1.y;
    CNFGPenX = i.rect.p1.x + 10;
    CNFGPenY = i.rect.p1.y + rect_height / 2 - text_height;
    if (i.focused)
        CNFGColor(COLOR(0xFF0000FF));
    else
        CNFGColor(i.rect.color);
    CNFGTackRectangle(i.rect.p1.x, i.rect.p1.y, i.rect.p2.x, i.rect.p2.y);
    CNFGColor(i.text.color);
    CNFGDrawText(i.text.string.data, i.text.font_size);
}

void DrawInputFields()
{
    for (size_t i = 0; i < input_fields.count; ++i)
        DrawInputField(input_fields.arr[i]);
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

    char str[64] = {0};
    char str2[64] = {0};
    InputField fields[16] = {0};

    input_fields.arr = fields;

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
            .offset = 0,
        },
        .focused = 1,
        .oninput = oninput_i,
    };

    InputField i2 = {
        .rect = (Rect){
            .color = WHITE,
            .p1 = (Point){
                .x = 0,
                .y = 50,
            },
            .p2 = (Point){
                .x = w,
                .y = 90,
            },
        },
        .text = (Text){
            .string = (String){
                .data = str2,
                .length = 0,
            },
            .color = BLACK,
            .font_size = 5,
            .offset = 0,
        },
        .focused = 0,
        .oninput = oninput_i,
    };

    add_input_field(i);
    add_input_field(i2);

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

        DrawInputFields();

        CNFGSwapBuffers();
    }

    return 0;
}
