#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOURCES "pm_console.c"
#define FLAGS "-Wall", "-Wextra", "-pedantic", "-std=c99", "-Isource/include/core"
#define MSVC_FLAGS "/W3", "/FC", "/nologo", "/Isource/include/core", "/link", "User32.lib"
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
    int test = 0, debug = 0;
    for (int i = 0; i < argc; ++i)
    {
        if (!test && strcmp(argv[i], "test") == 0)
            test = 1;
        if (!debug && strcmp(argv[i], "debug") == 0)
            debug = 1;
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
    int msvc = cc == NULL || strcmp(cc, "cl") == 0 || strcmp(cc, "cl.exe") == 0;
    if (test)
    {
        if (msvc)
        {
            CMD("cl.exe", "/DTEST", "/DEBUG", "/ZI", "/Fetest.exe", "/Od",
                "test.c", SOURCES, MSVC_FLAGS);
        }
        else
        {
            CMD(cc, "-D_GNU_SOURCE", "-DTEST", "test.c", SOURCES, FLAGS, "-lUser32", "-otest",
                "-O0", "-ggdb");
        }
        RUN("test");
    }
    else
    {
        if (debug)
        {
            CMD(cc, "-D_GNU_SOURCE", "source/core/pm_main.c", SOURCES, FLAGS, "-o" OUTPUT, "-O0", "-ggdb");
            return 0;
        }
        if (msvc)
        {
            pm_version[0] = '/';
            CMD("cl.exe", "/D_CRT_SECURE_NO_WARNINGS", "/DNDEBUG", pm_version, "/Fe" OUTPUT, "/O2", "source/core/pm_main.c", SOURCES,
                MSVC_FLAGS);
        }
        else
        {
            pm_version[0] = '-';
            CMD(cc, "-DNDEBUG", "-D_GNU_SOURCE", pm_version, "source/core/pm_main.c", SOURCES, FLAGS, "-lUser32", "-o" OUTPUT,
                "-O3");
        }
    }
#else
    if (cc == NULL)
    {
        cc = "gcc";
    }
    if (debug)
    {
        CMD(cc, "-D_GNU_SOURCE", "source/core/pm_main.c", SOURCES, FLAGS, "-o" OUTPUT, "-O0", "-ggdb");
        return 0;
    }
    if (test)
    {
        CMD(cc, "-D_GNU_SOURCE", "-DTEST", "test.c", SOURCES, FLAGS, "-otest", "-O0", "-ggdb");
        RUN("test");
    }
    else
    {
        pm_version[0] = '-';
        CMD(cc, "-DNDEBUG", "-D_GNU_SOURCE", pm_version, "source/core/pm_main.c", SOURCES, FLAGS, "-o" OUTPUT, "-O3");
    }
#endif
    return 0;
}
