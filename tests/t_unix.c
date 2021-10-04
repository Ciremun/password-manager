#include "t_common.h"

int run_test_in_fork(Args *a)
{
    uint8_t *aes_key = get_key();
    pid_t pid = 0;
    int status;

    pid = fork();
    if (pid == 0)
    {
        run(aes_key, a->argc, a->argv);
        free(aes_key);
        exit(0);
    }
    else
    {
        wait(&status);
        return WEXITSTATUS(status);
    }
}
