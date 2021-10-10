#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "common.h"
#undef exit

#define AES_KEY "test_aes_key!@#$%^&*();'"
#define ARGS(...) fill_args(__VA_ARGS__, NULL)
#define TEST_DATA_FILE "test.txt"
#define TEST_KEY_FILE "key_file.txt"
#define TEST_KEY_FLAG_DATA "test_key_flag_data"
#define TEST_KEY_FILE_FLAG_DATA "test_key_file_flag_data"
#define TEST_INPUT_FLAG_DATA "test_input_flag_data"

typedef struct Test Test;

typedef enum
{
    NO_FLAG = 0,
    DATA,
    DATA_FILE,
    LABEL,
    FIND_LABEL,
    DELETE_LABEL,
    GENERATE_PASSWORD,
    COPY_TO_CLIPBOARD,
    KEY,
    KEY_FILE,
    INPUT_
} Type;

typedef struct
{
    uint8_t *key;
    int argc;
    char **argv;
} Args;

struct Test
{
    void (*f)(Test *t);
    Args a;
    Type t;
    const char *desc;
};

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
char *data_store = 0;
size_t failed_tests_count = 0;

void reset_key(Args *a)
{
    a->key = calloc(1, 128);
    memcpy(a->key, AES_KEY, sizeof(AES_KEY));
}

const char *test_catergory(Type t)
{
    switch (t)
    {
    case NO_FLAG:
        return "No Flag";
    case DATA:
        return "Data";
    case DATA_FILE:
        return "Data File";
    case LABEL:
        return "Label";
    case FIND_LABEL:
        return "Find Label";
    case DELETE_LABEL:
        return "Delete Label";
    case GENERATE_PASSWORD:
        return "Generate Password";
    case COPY_TO_CLIPBOARD:
        return "Copy To Clipboard";
    case KEY:
        return "Key";
    case KEY_FILE:
        return "Key File";
    case INPUT_:
        return "Input";
    default:
        exit(1);
    }
}

#ifdef _WIN32
int run_from_win_thread(Args *a) { return run(a->key, a->argc, a->argv); }

int run_test_in_fork(Args *a)
{
    HANDLE hThread = CreateThread(
        NULL, 0, (LPTHREAD_START_ROUTINE)run_from_win_thread, a, 0, NULL);
    DWORD exit_status;
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &exit_status);
    CloseHandle(hThread);
    return (int)exit_status;
}
#else
int run_test_in_fork(Args *a)
{
    pid_t pid = 0;
    int status;

    pid = fork();
    if (pid == 0)
    {
        run(a->key, a->argc, a->argv);
        exit(0);
    }
    else
    {
        wait(&status);
        return WEXITSTATUS(status);
    }

    return 0;
}
#endif // _WIN32

Args fill_args(char *first, ...)
{
    va_list args_list;
    Args args = {.argc = 2, .argv = calloc(1, 256)};
    args.argv[1] = strdup(first);

    va_start(args_list, first);
    for (char *next = va_arg(args_list, char *); next != 0;
         next = va_arg(args_list, char *), args.argc++)
        args.argv[args.argc] = strdup(next);
    va_end(args_list);

    return args;
}

void free_argv(Args *a)
{
    for (int i = 0; i < a->argc; i++)
        free(a->argv[i]);
    free(a->argv);
}

void exit_test_case(int exit_code)
{
#ifdef _WIN32
    ExitThread(exit_code);
#else
    exit(exit_code);
#endif // _WIN32
}

void test(int check, Test *t)
{
    if (!check)
    {
        failed_tests_count++;
        fprintf(stderr, "%-17.17s %-25.25s FAIL\n", test_catergory(t->t),
                t->desc);
    }
    else
        fprintf(stderr, "%-17.17s %-25.25s PASS\n", test_catergory(t->t),
                t->desc);
}

void test_no_flag_pm_data_exists(Test *t)
{
    write_file(DEFAULT_DATA_STORE, "wb", "");
    test(run_test_in_fork(&t->a) == 0, t);
    remove(DEFAULT_DATA_STORE);
#ifndef _WIN32
    free(t->a.key);
#endif // _WIN32
}

void test_no_flag_pm_data_doesnt_exist(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
    free(t->a.key);
}

void test_data_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) != 0, t);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_data_flag_not_empty(Test *t)
{
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char **lines = NULL;
    read_file(DEFAULT_DATA_STORE, &lines, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data
        = b64_decode_ex(lines[0], strlen(lines[0]), &decsize);

    reset_key(&t->a);
    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("data", (char *)decoded_data) == 0, t);

    for (size_t i = 0; i < nch; ++i)
        free(lines[i]);
    free(lines);
    free(decoded_data);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_data_flag_with_generate_password(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_data_flag_with_data_file(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_data_file_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_data_file_flag_empty_file(Test *t)
{
    write_file(TEST_DATA_FILE, "wb", "");
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    reset_key(&t->a);
    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("\n", (char *)decoded_data) == 0, t);

    free(t->a.key);
    free(decoded_data);
    free(data);
    remove(DEFAULT_DATA_STORE);
    remove(TEST_DATA_FILE);
}

void test_data_file_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
    free(t->a.key);
}

void test_data_file_flag_valid(Test *t)
{
    write_file(TEST_DATA_FILE, "wb", "test data file");
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    reset_key(&t->a);
    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("test data file\n", (char *)decoded_data) == 0, t);

    free(t->a.key);
    free(decoded_data);
    free(data);
    remove(DEFAULT_DATA_STORE);
    remove(TEST_DATA_FILE);
}

void test_data_file_flag_with_generate_password(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_label_flag_not_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_find_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_delete_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_generate_password_flag_empty(Test *t)
{
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    test(decsize >= 15, t);

    free(data);
    free(decoded_data);
    remove(DEFAULT_DATA_STORE);
}

void test_generate_password_flag_128_chars(Test *t)
{
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    reset_key(&t->a);
    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strlen((char *)decoded_data) == 128, t);

    free(decoded_data);
    free(data);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_key_flag_empty(Test *t)
{
    test(run(NULL, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_key_flag_valid(Test *t)
{
    run(NULL, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FLAG_DATA) == 0, t);

    free(data);
    free(decoded_data);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_key_flag_invalid(Test *t)
{
    run(NULL, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FLAG_DATA) != 0, t);

    free(data);
    free(decoded_data);
    free(t->a.key);
    remove(DEFAULT_DATA_STORE);
}

void test_key_file_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_key_file_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
    free(t->a.key);
}

void test_key_file_flag_valid(Test *t)
{
    FILE *f = fopen(TEST_KEY_FILE, "wb");
    if (f == NULL)
    {
        error(stderr, "error opening file %s\n", TEST_KEY_FILE);
        exit(1);
    }
    fprintf(f, "%s", AES_KEY);
    fclose(f);

    run(NULL, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FILE_FLAG_DATA) == 0, t);

    free(decoded_data);
    free(data);
    free(t->a.key);
    remove(TEST_KEY_FILE);
    remove(DEFAULT_DATA_STORE);
}

void test_key_file_flag_invalid(Test *t)
{
    FILE *f = fopen(TEST_KEY_FILE, "wb");
    if (f == NULL)
    {
        error(stderr, "error opening file %s\n", TEST_KEY_FILE);
        exit(1);
    }
    fprintf(f, "%s", "invalid_key");
    fclose(f);

    run(NULL, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(DEFAULT_DATA_STORE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FILE_FLAG_DATA) != 0, t);

    free(decoded_data);
    free(data);
    free(t->a.key);
    remove(TEST_KEY_FILE);
    remove(DEFAULT_DATA_STORE);
}

void test_input_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
    free(t->a.key);
}

void test_input_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
    free(t->a.key);
}

void test_input_flag_write_data(Test *t)
{
    run(t->a.key, t->a.argc, t->a.argv);

    size_t nch = 0;
    char *data = read_file_as_str(TEST_DATA_FILE, &nch);

    size_t decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    reset_key(&t->a);
    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_INPUT_FLAG_DATA) == 0, t);

    free(data);
    free(decoded_data);
}

void run_test(Test *t)
{
    reset_key(&t->a);
    t->f(t);
}

int main(void)
{
    Test tests[] = {
        {
            .t = NO_FLAG,
            .f = test_no_flag_pm_data_exists,
            .a = {.argc = 1, .argv = calloc(1, 256)},
            .desc = ".pm_data exists",
        },
        {
            .t = NO_FLAG,
            .f = test_no_flag_pm_data_doesnt_exist,
            .a = {.argc = 1, .argv = calloc(1, 256)},
            .desc = ".pm_data doesn't exist",
        },
        {
            .t = DATA,
            .f = test_data_flag_empty,
            .a = ARGS("-d"),
            .desc = "empty",
        },
        {
            .t = DATA,
            .f = test_data_flag_not_empty,
            .a = ARGS("-d", "data"),
            .desc = "not empty",
        },
        {
            .t = DATA,
            .f = test_data_flag_with_generate_password,
            .a = ARGS("-d", "data", "-gp"),
            .desc = "with generate password",
        },
        {
            .t = DATA,
            .f = test_data_flag_with_data_file,
            .a = ARGS("-d", "data", "-df", TEST_DATA_FILE),
            .desc = "with data file",
        },
        {
            .t = DATA_FILE,
            .f = test_data_file_flag_empty,
            .a = ARGS("-df"),
            .desc = "empty",
        },
        {
            .t = DATA_FILE,
            .f = test_data_file_flag_empty_file,
            .a = ARGS("-df", TEST_DATA_FILE),
            .desc = "empty file",
        },
        {
            .t = DATA_FILE,
            .f = test_data_file_flag_non_existent_file,
            .a = ARGS("-df", TEST_DATA_FILE),
            .desc = "non-existent",
        },
        {
            .t = DATA_FILE,
            .f = test_data_file_flag_valid,
            .a = ARGS("-df", TEST_DATA_FILE),
            .desc = "valid",
        },
        {
            .t = DATA_FILE,
            .f = test_data_file_flag_with_generate_password,
            .a = ARGS("-df", TEST_DATA_FILE, "-gp"),
            .desc = "with generate password",
        },
        {
            .t = LABEL,
            .f = test_label_flag_empty,
            .a = ARGS("-l"),
            .desc = "empty",
        },
        {
            .t = LABEL,
            .f = test_label_flag_not_empty,
            .a = ARGS("-l", "label"),
            .desc = "not empty",
        },
        {
            .t = FIND_LABEL,
            .f = test_find_label_flag_empty,
            .a = ARGS("-fl"),
            .desc = "empty",
        },
        {
            .t = DELETE_LABEL,
            .f = test_delete_label_flag_empty,
            .a = ARGS("-dl"),
            .desc = "empty",
        },
        {
            .t = GENERATE_PASSWORD,
            .f = test_generate_password_flag_empty,
            .a = ARGS("-gp"),
            .desc = "empty",
        },
        {
            .t = GENERATE_PASSWORD,
            .f = test_generate_password_flag_128_chars,
            .a = ARGS("-gp", "128"),
            .desc = "128 chars",
        },
        {
            .t = KEY,
            .f = test_key_flag_empty,
            .a = ARGS("-k"),
            .desc = "empty",
        },
        {
            .t = KEY,
            .f = test_key_flag_valid,
            .a = ARGS("-k", AES_KEY, "-d", TEST_KEY_FLAG_DATA),
            .desc = "valid",
        },
        {
            .t = KEY,
            .f = test_key_flag_invalid,
            .a = ARGS("-k", "invalid_key", "-d", TEST_KEY_FLAG_DATA),
            .desc = "invalid",
        },
        {
            .t = KEY_FILE,
            .f = test_key_file_flag_empty,
            .a = ARGS("-kf"),
            .desc = "empty",
        },
        {
            .t = KEY_FILE,
            .f = test_key_file_flag_non_existent_file,
            .a = ARGS("-kf", "keyfile_that_doesnt_exist.txt"),
            .desc = "non-existent",
        },
        {
            .t = KEY_FILE,
            .f = test_key_file_flag_valid,
            .a = ARGS("-kf", TEST_KEY_FILE, "-d", TEST_KEY_FILE_FLAG_DATA),
            .desc = "valid",
        },
        {
            .t = KEY_FILE,
            .f = test_key_file_flag_invalid,
            .a = ARGS("-kf", TEST_KEY_FILE, "-d", TEST_KEY_FILE_FLAG_DATA),
            .desc = "invalid",
        },
        {
            .t = INPUT_,
            .f = test_input_flag_empty,
            .a = ARGS("-i"),
            .desc = "empty",
        },
        {
            .t = INPUT_,
            .f = test_input_flag_non_existent_file,
            .a = ARGS("-i", "input_file_that_doesnt_exist.txt"),
            .desc = "non-existent",
        },
        {
            .t = INPUT_,
            .f = test_input_flag_write_data,
            .a = ARGS("-i", TEST_DATA_FILE, "-d", TEST_INPUT_FLAG_DATA),
            .desc = "write data",
        },
    };

    size_t tests_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < tests_count; i++)
    {
        run_test(&tests[i]);
        free_argv(&tests[i].a);
    }

    printf("\n(%zu / %zu) tests passed\n", tests_count - failed_tests_count,
           tests_count);
    return failed_tests_count >= 1;
}
