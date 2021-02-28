#include <stdarg.h>

#include "../src/io/common.h"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
uint8_t *aes_key = NULL;

void reset_key()
{
    memcpy(aes_key, "test_aes_key!@#$%^&*();'", 25);
}

void assert_t(int check, const char *test)
{
    if (!check)
        printf("%s FAIL\n", test);
    else
        printf("%s PASS\n", test);
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

void test_data_flag(void)
{
    int argc = 3;
    char **argv = fill_args(argc, "-d", "data");

    run(aes_key, argc, argv);
    reset_key();

    size_t nch = 0;
    char **lines = NULL;
    read_file(DATA_STORE, &lines, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(lines[0], strlen(lines[0]), &decsize);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    assert_t(strcmp("data", (char *)decoded_data) == 0, "\"-d data\"");

    free_argv(argc, argv);
}

void setup_test(void)
{
    reset_key();
    printf("setup\n");
}

void exit_test(void)
{
    reset_key();
    remove(DATA_STORE);
    printf("free\n");
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

    run_test(test_data_flag);

    return 0;
}
