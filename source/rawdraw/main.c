// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#ifdef _MSC_VER
#pragma comment(lib, "gdi32")
#pragma comment(lib, "User32")
#endif // _MSC_VER

#ifndef __wasm__
#include "rawdraw/vendor/os_generic.h"
#include <stdlib.h>
#endif // __wasm__

#define CNFG_IMPLEMENTATION
#include "rawdraw/vendor/rawdraw_sf.h"

#include "core/pm_util.h"
#include "rawdraw/rd_event.h"
#include "rawdraw/rd_ui.h"
#include "rawdraw/rd_util.h"

#define WINDOW_NAME "password-manager"

short w, h;
int paused = 0;

extern InputFields input_fields;

#ifndef __wasm__
extern String sync_remote_url;
#endif // __wasm__

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

#ifndef __wasm__
    sync_remote_url = (String){
        .data = (uint8_t *)getenv("PM_SYNC_REMOTE_URL"),
    };

    if (sync_remote_url.data != 0)
        sync_remote_url.length = strlen((char *)sync_remote_url.data);
#endif // __wasm__

    uint8_t str[64] = {0};
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
        .oninput = oninput,
    };

    add_input_field(i);

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
