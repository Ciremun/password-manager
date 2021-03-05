#include "t_common.h"

void exit_tests(void)
{
    ExitThread(0);
}

void exit_win_thread(void)
{
    ExitThread(last_exit_code);
}

void run_from_win_thread(Args *a)
{
    atexit(exit_win_thread);
    uint8_t *aes_key = get_key();
    int exit_code = run(aes_key, a->argc, a->argv);
    free(aes_key);
    ExitThread(exit_code);
}

DWORD run_test_in_win_thread(Args *a)
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_from_win_thread, a, 0, NULL);
    DWORD exit_status;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exit_status);
    CloseHandle(hThread);
    return exit_status;
}

void test_no_flag(void)
{
    int argc = 1;
    char **argv = calloc(1, 256);
    Args a = {.argc = argc, .argv = argv};
    assert_t(run_test_in_win_thread(&a) == 1, "void" TABS);
}
