#include "t_common.h"

void run_from_win_thread(Args *a)
{
    uint8_t *aes_key = get_key();
    run(aes_key, a->argc, a->argv);
    free(aes_key);
}

int run_test_in_fork(Args *a)
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run_from_win_thread, a, 0, NULL);
    DWORD exit_status;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exit_status);
    CloseHandle(hThread);
    return (int)exit_status;
}
