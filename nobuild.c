#include "src/io/common.h"
#undef PANIC

#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define SOURCES "src/aes.c", "src/b64/encode.c", "src/b64/decode.c", "src/b64/buffer.c", "src/io/common.c", "src/rand.c", "src/parse.c"
#define FLAGS "-Wall", "-Wextra", "-pedantic"
#define MSVC_FLAGS "/FC", "/nologo", "/link", "User32.lib"

#define PANIC_OVERWRITE_IF_FILE_EXISTS(PATH)                                                                     \
    do                                                                                                           \
    {                                                                                                            \
        if (PATH_EXISTS(PATH))                                                                                   \
        {                                                                                                        \
            PANIC("file '" PATH "' will be overwritten by tests, please move it outside of the root directory"); \
        }                                                                                                        \
    } while (0)

int main(int argc, char **argv)
{
    char *cc = getenv("cc");
    int   test = argc > 1 ? strcmp(argv[1], "test") == 0 ? 1 : 0 : 0;
    if (test)
    {
        PANIC_OVERWRITE_IF_FILE_EXISTS(DEFAULT_DATA_STORE);
        PANIC_OVERWRITE_IF_FILE_EXISTS("test.txt");
    }
#ifdef _WIN32
    int msvc = cc == NULL || strcmp(cc, "cl") == 0 || strcmp(cc, "cl.exe") == 0;
    if (test)
    {
        if (msvc)
        {
            CMD("cl.exe", "/Fetest.exe", "/Od", "tests/test.c", "tests/t_win.c", "src/io/win.c", SOURCES, MSVC_FLAGS);
        }
        else
        {
            CMD(cc, "tests/test.c", "tests/t_win.c", "src/io/win.c", SOURCES, FLAGS, "-lUser32", "-otest", "-O0", "-ggdb");
        }
        CMD(".\\test.exe");
    }
    else
    {
        if (msvc)
        {
            CMD("cl.exe", "/Fepm.exe", "/O2", "src/main.c", "src/io/win.c", SOURCES, MSVC_FLAGS);
        }
        else
        {
            CMD(cc, "src/main.c", "src/io/win.c", SOURCES, FLAGS, "-lUser32", "-opm", "-O3");
        }
    }
#else
    if (cc == NULL)
    {
        cc = "gcc";
    }
    if (test)
    {
        CMD(cc, "tests/test.c", "tests/t_unix.c", "src/io/unix.c", SOURCES, FLAGS, "-otest", "-O0", "-ggdb");
        CMD("./test");
    }
    else
    {
        CMD(cc, "src/main.c", "src/io/unix.c", SOURCES, FLAGS, "-opm", "-O3");
    }
#endif
    return 0;
}