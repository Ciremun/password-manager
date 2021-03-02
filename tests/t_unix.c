#include "t_common.h"

void exit_tests(void)
{
    exit(0);
}

void test_no_flag(void)
{
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
}
