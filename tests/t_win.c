#include "t_common.h"

void exit_tests(void)
{
    ExitThread(0);
}

void exit_win_thread(void)
{
    ExitThread(last_exit_code);
}

void run_from_win_thread(void)
{
    atexit(exit_win_thread);
    uint8_t *aes_key = get_key();
    int argc = 1;
    char **argv = calloc(1, 256);
    run(aes_key, argc, argv);
    free(aes_key);
}

void test_no_flag(void)
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_from_win_thread, NULL, 0, NULL);
    DWORD exit_status;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exit_status);
    assert_t(exit_status == 1, "void"TABS);
    CloseHandle(hThread);
}
