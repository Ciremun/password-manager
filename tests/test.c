#include <stdarg.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#else
#include <windows.h>
#endif

#include "../src/io/common.h"

#define TABS "\t"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
uint8_t *aes_key = NULL;

#ifdef _WIN32
void exit_win_thread(void)
{
    ExitThread(last_exit_code);
}

void run_from_win_thread(void)
{
    atexit(exit_win_thread);
    int argc = 1;
    char **argv = calloc(1, 256);
    run(aes_key, argc, argv);
}
#endif

void exit_tests(void)
{
    #ifdef _WIN32
    ExitThread(0);
    #else
    exit(0);
    #endif
}

void reset_key()
{
    memcpy(aes_key, "test_aes_key!@#$%^&*();'", 25);
}

void assert_t(int check, const char *test)
{
    if (!check)
        fprintf(stderr, "%s FAIL\n", test);
    else
        fprintf(stderr, "%s PASS\n", test);
}

char **fill_args(int argc, ...)
{
    va_list ap;
    int n = 1;
    char **argv = calloc(1, 256);
    argc--;

    va_start(ap, argc);
    while (argc--)
    {
        argv[n] = strdup(va_arg(ap, char *));
        n++;
    }
    va_end(ap);

    return argv;
}

void free_argv(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    free(argv);
}

void test_no_flag(void)
{
    // create separate files for Unix and Win
    #ifdef _WIN32
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_from_win_thread, NULL, 0, NULL);
    LPDWORD exit_status;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exit_status);
    assert_t((int)exit_status == 1, "void"TABS);
    CloseHandle(hThread);
    #else

    pid_t pid = 0;
    int status;

    pid = fork();
    if (pid == 0)
    {
        int argc = 1;
        char **argv = calloc(1, 256);
        run(aes_key, argc, argv);
    }
    if (pid > 0)
    {
        pid = wait(&status);
        assert_t(WEXITSTATUS(status) == 1, "void"TABS);
    }
    #endif // _WIN32
}

void test_data_flag(void)
{
    int argc = 2;
    char **argv = fill_args(argc, "-d");

    assert_t(run(aes_key, argc, argv) != 0, "-d"TABS);
    reset_key();
    free_argv(argc, argv);

    argc = 3;
    argv = fill_args(argc, "-d", "data");

    run(aes_key, argc, argv);
    reset_key();
    free_argv(argc, argv);

    size_t nch = 0;
    char **lines = NULL;
    read_file(DATA_STORE, &lines, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(lines[0], strlen(lines[0]), &decsize);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    assert_t(strcmp("data", (char *)decoded_data) == 0, "-d data"TABS);
}

void setup_test(void)
{
    reset_key();
}

void exit_test(void)
{
    reset_key();
    remove(DATA_STORE);
}

void run_test(void (*test)(void))
{
    setup_test();
    test();
    exit_test();
}

int main(void)
{
    aes_key = calloc(1, 32);

    close(1);

    run_test(test_no_flag);
    run_test(test_data_flag);

    exit_tests();
}
