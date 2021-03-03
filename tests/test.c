#include "t_common.h"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

void (*tests[])(void) = {
    test_no_flag,
    test_data_flag,
    test_data_file_flag,
    test_label_flag
};

size_t tests_count = sizeof(tests) / sizeof(tests[0]);

uint8_t *get_key(void)
{
    uint8_t *aes_key = calloc(1, 32);
    reset_key(aes_key);
    return aes_key;
}

void reset_key(uint8_t *aes_key)
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

void test_data_flag(void)
{
    uint8_t *aes_key = get_key();
    int argc = 2;
    char **argv = fill_args(argc, "-d");

    assert_t(run(aes_key, argc, argv) != 0, "-d" TABS);
    reset_key(aes_key);
    free_argv(argc, argv);

    argc = 3;
    argv = fill_args(argc, "-d", "data");

    run(aes_key, argc, argv);
    reset_key(aes_key);
    free_argv(argc, argv);

    size_t nch = 0;
    char **lines = NULL;
    read_file(DATA_STORE, &lines, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(lines[0], strlen(lines[0]), &decsize);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    assert_t(strcmp("data", (char *)decoded_data) == 0, "-d data" TABS);
}

void test_data_file_flag(void)
{
    uint8_t *aes_key = get_key();
    int argc = 2;
    char **argv = fill_args(argc, "-df");

    assert_t(run(aes_key, argc, argv) == 1, "-df" TABS);
    free_argv(argc, argv);

    write_file("test.txt", "w", "test data file");

    argc = 3;
    argv = fill_args(argc, "-df", "test.txt");

    run(aes_key, argc, argv);
    reset_key(aes_key);
    free_argv(argc, argv);

    remove("test.txt");

    size_t nch = 0;
    char *data = read_file_as_str(DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    assert_t(strcmp("test data file\n", (char *)decoded_data) == 0, "-df test.txt\t");
}

void test_label_flag(void)
{
    uint8_t *aes_key = get_key();
    int argc = 2;
    char **argv = fill_args(argc, "-l");

    assert_t(run(aes_key, argc, argv) == 1, "-l" TABS);
    reset_key(aes_key);
    free_argv(argc, argv);

    argc = 3;
    argv = fill_args(argc, "-l", "label");

    assert_t(run(aes_key, argc, argv) == 1, "-l label\t");
    free_argv(argc, argv);
    free(aes_key);
}

void setup_test(void)
{
}

void exit_test(void)
{
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
    close(1);

    for (size_t i = 0; i < tests_count; i++)
    {
        run_test(tests[i]);
    }

    exit_tests();
}
