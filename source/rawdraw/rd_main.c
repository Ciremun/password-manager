// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#ifdef _MSC_VER
#pragma comment(lib, "gdi32")
#pragma comment(lib, "User32")
#endif // _MSC_VER

#ifndef __wasm__
#include "rawdraw/vendor/os_generic.h"
#include "rawdraw/rd_xcrypt.h"
#include "core/pm_parse.h"
#include "core/pm_xcrypt.h"
#include "core/pm_io.h"
#include <stdlib.h>
#else
#include "wasm/wasm_xcrypt.h"
#endif // __wasm__

#define CNFG_IMPLEMENTATION
#include "rawdraw/vendor/rawdraw_sf.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "rawdraw/vendor/stb_sprintf.h"

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
extern char *data_store;
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
    w = 1000;
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
#ifdef __ANDROID__
    char path[1024] = {0};
    sprintf(path, "%s/" DEFAULT_DATA_STORE, AndroidGetExternalFilesDir());
    data_store = path;
#else
    data_store = DEFAULT_DATA_STORE;
#endif // __ANDROID__

    if (sync_remote_url.data != 0)
        sync_remote_url.length = strlen((char *)sync_remote_url.data);
#endif // __wasm__

    input_fields.arr = (InputField *)calloc(64, sizeof(InputField));

    uint8_t aes_key[32] = {0};
    memcpy(aes_key, "secret_key", 10);

    char hello[] = "tsodinSleep";
    // if (!AndroidHasPermissions("WRITE_EXTERNAL_STORAGE"))
        // AndroidRequestAppPermissions("WRITE_EXTERNAL_STORAGE");
#ifndef __wasm__
    rd_encrypt_and_write(PM_STR(hello), aes_key);
#endif // __wasm__
    decrypt_and_draw(aes_key);

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
