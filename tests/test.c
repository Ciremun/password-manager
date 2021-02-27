#include <stdarg.h>

#include "../src/io/common.h"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
uint8_t *aes_key = NULL;

void assert_t(int check, const char *test)
{
    if (!check)
        printf("%s FAIL\n", test);
    else
        printf("%s PASS\n", test);
}

void fill_args(int *argc, char **argv, ...)
{
    va_list ap;
    int n = 1;

    va_start(ap, argv);
    for (int i = 0; i < 2; i++)
    {
        char *arg = (char *)va_arg(ap, char *);
        argv[n] = strdup(arg);
        n++;
    }
    va_end(ap);
    *argc = n;
}

void test_data_flag()
{
    char **argv = calloc(1, 256);
    int argc;
    fill_args(&argc, argv, "-d", "data");
    assert_t(run(aes_key, argc, argv) == 0, "\"-d data\"");
}

void setup_test()
{
    printf("setup\n");
}

void exit_test()
{
    printf("free\n");
}

void run_test(void (*test)(void))
{
    setup_test();
    test();
    exit_test();
}

int main()
{
    aes_key = calloc(1, 32);
    memcpy(aes_key, "test_aes_key!@#$%^&*();'", 25);

    run_test(test_data_flag);

    return 0;
}
