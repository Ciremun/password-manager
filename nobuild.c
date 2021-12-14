#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER) && !defined(__clang__)
#define INCLUDES "/Isource/include"
#else
#define INCLUDES "-Isource/include"
#endif // defined(_MSC_VER) && !defined(__clang__)

#define CONSOLE_SOURCES "source/pm_console.c"
#define WASM_SOURCES
#define CORE_SOURCES "source/core/pm_aes.c", "source/core/pm_b64.c", "source/core/pm_io.c", "source/core/pm_parse.c", "source/core/pm_rand.c", "source/core/pm_sync.c", "source/core/pm_xcrypt.c"
#define RAWDRAW_SOURCES "source/rawdraw/rd_event.c", "source/rawdraw/rd_ui.c", "source/rawdraw/rd_util.c", "source/rawdraw/rd_xcrypt.c"
#define CFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c99", "-D_GNU_SOURCE"
#define MSVC_CFLAGS "/W3", "/FC", "/nologo", "/D_CRT_SECURE_NO_WARNINGS"
#define DEBUG_CFLAGS "-O0", "-ggdb"
#define RELEASE_CFLAGS "-O3", "-g0"
#define DEFAULT_DATA_STORE ".pm_data"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if defined(_WIN32)
#define OS "win"
#elif defined(__ANDROID__)
#define OS "android"
#elif defined(__APPLE__)
#define OS "macos"
#elif defined(__linux__)
#define OS "linux"
#elif defined(__unix__)
#define OS "unix"
#else
#define OS "unknown"
#endif // OS

#if defined(__GNUC__) && defined(__llvm__) && defined(__clang__)
#define CC                                                          \
    "clang-" STR(__clang_major__) "." STR(__clang_minor__) "." STR( \
        __clang_patchlevel__)
#elif defined(_MSC_VER) && !defined(__clang__)
#define CC "msvc-" STR(_MSC_VER)
#elif defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#define CC \
    "gcc-" STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#else
#define CC "unknown"
#endif // CC

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH "x86_64"
#elif defined(__aarch64__)
#define ARCH "aarch64"
#elif defined(__arm__) || defined(_M_ARM)
#define ARCH "arm"
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(_M_IX86) || defined(_X86_)
#define ARCH "intel_x86"
#else
#define ARCH "unknown"
#endif // ARCH

#if defined(_WIN32)
#define OUTPUT "pm-" OS "-" CC "-" ARCH ".exe"
#define RUN(e) CMD(e ".exe")
#else
#define OUTPUT "pm-" OS "-" CC "-" ARCH
#define RUN(e) CMD("./" e)
#endif // _WIN32

#define PANIC_OVERWRITE_IF_FILE_EXISTS(PATH)                                   \
    do                                                                         \
    {                                                                          \
        if (PATH_EXISTS(PATH))                                                 \
        {                                                                      \
            PANIC("file '" PATH "' will be overwritten by tests, please move " \
                  "it outside of the root directory");                         \
        }                                                                      \
    } while (0)

char pm_version[32] = "?DPM_VERSION=unknown";

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    char *cc = getenv("cc");
#ifdef _WIN32
    int msvc = cc == NULL || strcmp(cc, "cl") == 0 || strcmp(cc, "cl.exe") == 0;
#else
    if (cc == NULL)
        cc = "cc";
#endif // _WIN32
    int test = 0, debug = 0, wasm = 0, rawdraw = 0, run = 0;
    for (int i = 0; i < argc; ++i)
    {
        if (!test && strcmp(argv[i], "test") == 0)
            test = 1;
        if (!debug && strcmp(argv[i], "debug") == 0)
            debug = 1;
        if (!wasm && strcmp(argv[i], "wasm") == 0)
            wasm = 1;
        if (!rawdraw && strcmp(argv[i], "rawdraw") == 0)
            rawdraw = 1;
        if (!run && strcmp(argv[i], "run") == 0)
            run = 1;
    }
    if (wasm)
    {
#ifdef _WIN32
        printf("wasm build is not supported on Windows\n");
        return 1;
#else
        chdir("source/rawdraw/wasm");
        CMD("cc", "-o", "subst", "source/tools/subst.c");
        CMD("cc", "-o", "nn", "source/tools/nn.c");
        CMD("clang", "-DWASM", "-nostdlib", "--target=wasm32", "-I../../include", "-Iinclude", "../rd_event.c", "../rd_ui.c", "../rd_util.c", "../main.c", "-flto", "-Oz", "-Wl,--lto-O3", "-Wl,--no-entry", "-Wl,--allow-undefined", "-Wl,--import-memory", "-o", "main.wasm");
        CMD("wasm-opt", "--asyncify", "--pass-arg=asyncify-imports@bynsyncify.*", "--pass-arg=asyncify-ignore-indirect", "-Oz", "main.wasm", "-o", "main.wasm");
        CHAIN(CHAIN_CMD("cat", "main.wasm"), CHAIN_CMD("base64"), CHAIN_CMD("./nn"), OUT("blob_b64"));
        CMD("./subst", "source/template.js", "-s", "-f", "BLOB", "blob_b64", "-o", "mid.js");
        CMD("terser", "-ecma 2017", "-d", "RAWDRAW_USE_LOOP_FUNCTION=false", "-d", "RAWDRAW_NEED_BLITTER=true", "mid.js", "-o", "opt.js");
        remove("mid.js");
        remove("blob_b64");
        CMD("./subst", "source/template.ht", "-s", "-f", "JAVASCRIPT_DATA", "opt.js", "-o", "../../../index.html");
        return 0;
#endif // _WIN32
    }
    if (rawdraw)
    {
#ifdef _WIN32
        if (msvc)
            CMD("cl", MSVC_CFLAGS, "/DRAWDRAW", "/O2", "/DNDEBUG", "/Isource/include", "/Fe:pm-gui.exe", "source/rawdraw/main.c", RAWDRAW_SOURCES, CORE_SOURCES);
        else
        {
            if (debug)
                CMD(cc, CFLAGS, INCLUDES, DEBUG_CFLAGS, "-o", "pm-gui", "source/rawdraw/main.c", RAWDRAW_SOURCES, CORE_SOURCES, "-lUser32", "-lGdi32");
            else
                CMD(cc, CFLAGS, "-DNDEBUG", INCLUDES, RELEASE_CFLAGS, "-o", "pm-gui", "source/rawdraw/main.c", RAWDRAW_SOURCES, CORE_SOURCES, "-lUser32", "-lGdi32");
        }
#else
        if (debug)
            CMD(cc, CFLAGS, INCLUDES, DEBUG_CFLAGS, "-o", "pm-gui", "source/rawdraw/main.c", RAWDRAW_SOURCES, CORE_SOURCES, "-lX11");
        else
            CMD(cc, CFLAGS, "-DNDEBUG", INCLUDES, RELEASE_CFLAGS, "-o", "pm-gui", "source/rawdraw/main.c", RAWDRAW_SOURCES, CORE_SOURCES, "-lX11");
#endif // _WIN32
        if (run)
            RUN("pm-gui");
        return 0;
    }
    if (test)
    {
        PANIC_OVERWRITE_IF_FILE_EXISTS(DEFAULT_DATA_STORE);
        PANIC_OVERWRITE_IF_FILE_EXISTS("test.txt");
        PANIC_OVERWRITE_IF_FILE_EXISTS("key_file.txt");
    }
    FILE *git_heads_master = fopen(".git/refs/heads/master", "rb");
    if (git_heads_master == 0)
        printf("error opening .git/refs/heads/master\n");
    if (git_heads_master)
    {
        int i = 13;
        while (i < 20)
            pm_version[i++] = fgetc(git_heads_master);
        pm_version[i] = '\0';
    }
    if (git_heads_master)
        fclose(git_heads_master);
#if defined(PM_CROSSCOMPILING)
        // nothing
#elif defined(_WIN32)
    if (test)
    {
        if (msvc)
        {
            CMD("cl.exe", MSVC_CFLAGS, INCLUDES, "/DTEST", "/DEBUG", "/ZI", "/Fetest.exe", "/Od",
                "test.c", CONSOLE_SOURCES);
        }
        else
        {
            CMD(cc, "-DTEST", "test.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-lUser32", "-otest",
                DEBUG_CFLAGS);
        }
        RUN("test");
    }
    else
    {
        if (debug)
        {
            if (msvc)
            {
                printf("MSVC debug build is not supported\n");
                return 1;
            }
            CMD(cc, "source/core/main.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-o" OUTPUT, DEBUG_CFLAGS);
            return 0;
        }
        if (msvc)
        {
            pm_version[0] = '/';
            CMD("cl.exe", MSVC_CFLAGS, INCLUDES, "/DNDEBUG", pm_version, "/Fe" OUTPUT, "/O2", "source/core/main.c", CONSOLE_SOURCES);
        }
        else
        {
            pm_version[0] = '-';
            CMD(cc, "-DNDEBUG", pm_version, "source/core/main.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-lUser32", "-o" OUTPUT,
                RELEASE_CFLAGS);
        }
    }
#else
    if (debug)
    {
        CMD(cc, "source/core/main.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-o" OUTPUT, DEBUG_CFLAGS);
        return 0;
    }
    if (test)
    {
        CMD(cc, "-DTEST", "test.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-otest", DEBUG_CFLAGS);
        RUN("test");
    }
    else
    {
        pm_version[0] = '-';
        CMD(cc, "-DNDEBUG", pm_version, "source/core/main.c", CONSOLE_SOURCES, CFLAGS, INCLUDES, "-o" OUTPUT, RELEASE_CFLAGS);
    }
#endif
    return 0;
}
