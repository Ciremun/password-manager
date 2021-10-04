#include "src/io/common.h"
#undef PANIC

#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define SOURCES "src/sync.c", "src/aes.c", "src/b64/encode.c", "src/b64/decode.c", "src/b64/buffer.c", "src/io/common.c", "src/rand.c", "src/parse.c"
#define FLAGS "-Wall", "-Wextra", "-pedantic"
#define MSVC_FLAGS "/FC", "/nologo", "/link", "User32.lib"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if   defined(_WIN32)
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

#if   defined(__GNUC__) && defined(__llvm__) && defined(__clang__)
    #define CC "clang-" STR(__clang_major__) "." STR(__clang_minor__) "." STR(__clang_patchlevel__)
#elif defined(_MSC_VER) && !defined(__clang__)
    #define CC "msvc-" STR(_MSC_VER)
#elif defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
    #define CC "gcc-" STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#else
    #define CC "unknown"
#endif // CC

#if   defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
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
#else
#define OUTPUT "pm-" OS "-" CC "-" ARCH
#endif // _WIN32

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
    GO_REBUILD_URSELF(argc, argv);

    char *cc = getenv("cc");
    int   test = argc > 1 ? strcmp(argv[1], "test") == 0 ? 1 : 0 : 0;
    if (test)
    {
        PANIC_OVERWRITE_IF_FILE_EXISTS(DEFAULT_DATA_STORE);
        PANIC_OVERWRITE_IF_FILE_EXISTS("test.txt");
    }
    FILE *version_header = fopen("src/version.h", "wb");
    if (version_header == 0)
        printf("error opening src/version.h");
    FILE *git_heads_master = fopen(".git/refs/heads/master", "rb");
    if (git_heads_master == 0)
        printf("error opening .git/refs/heads/master");
    if (version_header && git_heads_master)
    {
        fprintf(version_header, "%s", "#define PM_VERSION \"");
        for (int i = 0; i < 7; ++i)
            fputc(fgetc(git_heads_master), version_header);
        fprintf(version_header, "%c\n", '"');
    }
    if (version_header)
        fclose(version_header);
    if (git_heads_master)
        fclose(git_heads_master);
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
            CMD("cl.exe", "/Fe"OUTPUT, "/O2", "src/main.c", "src/io/win.c", SOURCES, MSVC_FLAGS);
        }
        else
        {
            CMD(cc, "src/main.c", "src/io/win.c", SOURCES, FLAGS, "-lUser32", "-o"OUTPUT, "-O3");
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
        CMD(cc, "src/main.c", "src/io/unix.c", SOURCES, FLAGS, "-o"OUTPUT, "-O3");
    }
#endif
    return 0;
}