#include <assert.h>
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

#define ARGS(...)                       fill_args(__VA_ARGS__, NULL)
#define AES_KEY                         "test_aes_key!@#$%^&*();'"
#define TEST_DATA_FILE                  "test.txt"
#define TEST_KEY_FILE                   "key_file.txt"
#define TEST_KEY_FLAG_DATA              "test_key_flag_data"
#define TEST_KEY_FILE_FLAG_DATA         "test_key_file_flag_data"
#define TEST_INPUT_FLAG_DATA            "test_input_flag_data"
#define TEST_LABEL_FLAG_LABEL           "test_label_flag_label"
#define TEST_LABEL_FLAG_DATA            "test_label_flag_data"
#define TEST_LABEL_FLAG_REPLACE_DATA    "test_label_flag_replace_data"
#define NO_FLAG_CATEGORY_NAME           "No Flag"
#define DATA_CATEGORY_NAME              "Data"
#define DATA_FILE_CATEGORY_NAME         "Data File"
#define LABEL_CATEGORY_NAME             "Label"
#define FIND_LABEL_CATEGORY_NAME        "Find Label"
#define DELETE_LABEL_CATEGORY_NAME      "Delete Label"
#define GENERATE_PASSWORD_CATEGORY_NAME "Generate Password"
#define COPY_CATEGORY_NAME              "Copy To Clipboard"
#define KEY_CATEGORY_NAME               "Key"
#define KEY_FILE_CATEGORY_NAME          "Key File"
#define INPUT_CATEGORY_NAME             "Input"

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
    Type        t;
    const char *n;
} Category;

typedef struct
{
    uint8_t *key;
    int      argc;
    char   **argv;
} Args;

struct Test
{
    Args        a;
    Category    c;
    const char *d;
    void (*f)(Test *t);
};

extern Memory  g_mem;
struct AES_ctx ctx;
uint8_t        aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
char          *data_store = 0;
size_t         failed_tests_count = 0;

char *read_file_as_str_test(const char *fp, size_t *nch)
{
    FILE *f = fopen(fp, "rb");
    if (f == NULL)
        PANIC_OPEN_FILE(fp);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char  *str = (char *)malloc(size + 1);
    fseek(f, 0, SEEK_SET);
    fread(str, 1, size, f);
    str[size] = '\0';
    fclose(f);
    if (nch != NULL)
        *nch = size;
    return str;
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
    int   status;

    pid = fork();
    if (pid == 0)
    {
        run(a->key, a->argc, a->argv);
        exit(0);
    }
    else if (pid == -1)
    {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
        return -1;
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
    Args    args = {.argc = 2, .argv = calloc(1, 256)};
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
    mem_free(&g_mem);
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
        fprintf(stderr, "%-17.17s %-25.25s FAIL\n", t->c.n, t->d);
    }
    else
        fprintf(stderr, "%-17.17s %-25.25s PASS\n", t->c.n, t->d);
}

void test_no_flag_pm_data_exists(Test *t)
{
    write_file(DEFAULT_DATA_STORE, "wb", "");
    test(run_test_in_fork(&t->a) == 0, t);
    remove(DEFAULT_DATA_STORE);
}

void test_no_flag_pm_data_doesnt_exist(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
}

void test_data_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) != 0, t);
    remove(DEFAULT_DATA_STORE);
}

void test_data_flag_valid(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char **lines = NULL;
    read_file(DEFAULT_DATA_STORE, &lines, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data
        = b64_decode_ex(lines[0], strlen(lines[0]), &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("data", (char *)decoded_data) == 0, t);

    for (size_t i = 0; i < nch; ++i)
        free(lines[i]);
    free(lines);
    free(decoded_data);

    remove(DEFAULT_DATA_STORE);
}

void test_data_flag_with_generate_password(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_data_flag_with_data_file(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_data_file_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);

    remove(DEFAULT_DATA_STORE);
}

void test_data_file_flag_empty_file(Test *t)
{
    write_file(TEST_DATA_FILE, "wb", "");
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("\n", (char *)decoded_data) == 0, t);

    free(decoded_data);
    remove(DEFAULT_DATA_STORE);
    remove(TEST_DATA_FILE);
}

void test_data_file_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
}

void test_data_file_flag_valid(Test *t)
{
    write_file(TEST_DATA_FILE, "wb", "test data file");
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("test data file\n", (char *)decoded_data) == 0, t);

    free(decoded_data);
    remove(DEFAULT_DATA_STORE);
    remove(TEST_DATA_FILE);
}

void test_data_file_flag_with_generate_password(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_label_flag_valid(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp(TEST_LABEL_FLAG_LABEL " " TEST_LABEL_FLAG_DATA,
                (char *)decoded_data)
             == 0,
         t);

    free(decoded_data);
}

void test_label_flag_replace(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp(TEST_LABEL_FLAG_LABEL " " TEST_LABEL_FLAG_REPLACE_DATA,
                (char *)decoded_data)
             == 0,
         t);

    free(decoded_data);
}

void test_delete_label_flag_doesnt_exist(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp(TEST_LABEL_FLAG_LABEL " " TEST_LABEL_FLAG_REPLACE_DATA,
                (char *)decoded_data)
             == 0,
         t);

    free(decoded_data);
}

void test_delete_label_flag_exists(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp("", (char *)decoded_data) == 0, t);

    free(decoded_data);
    remove(DEFAULT_DATA_STORE);
}

void test_find_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_delete_label_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_generate_password_flag_empty(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    test(decsize >= 15, t);

    free(decoded_data);
    remove(DEFAULT_DATA_STORE);
}

void test_generate_password_flag_128_chars(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strlen((char *)decoded_data) == 128, t);

    free(decoded_data);

    remove(DEFAULT_DATA_STORE);
}

void test_key_flag_empty(Test *t)
{
    test(run(NULL, t->a.argc, t->a.argv) == 1, t);
}

void test_key_flag_valid(Test *t)
{
    assert(run(NULL, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FLAG_DATA) == 0, t);

    free(decoded_data);

    remove(DEFAULT_DATA_STORE);
}

void test_key_flag_invalid(Test *t)
{
    assert(run(NULL, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FLAG_DATA) != 0, t);

    free(decoded_data);

    remove(DEFAULT_DATA_STORE);
}

void test_key_file_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_key_file_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
}

void test_key_file_flag_valid(Test *t)
{
    FILE *f = fopen(TEST_KEY_FILE, "wb");
    if (f == NULL)
        PANIC_OPEN_FILE(TEST_KEY_FILE);
    fprintf(f, "%s", AES_KEY);
    fclose(f);

    assert(run(NULL, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FILE_FLAG_DATA) == 0, t);

    free(decoded_data);

    remove(TEST_KEY_FILE);
    remove(DEFAULT_DATA_STORE);
}

void test_key_file_flag_invalid(Test *t)
{
    FILE *f = fopen(TEST_KEY_FILE, "wb");
    if (f == NULL)
        PANIC_OPEN_FILE(TEST_KEY_FILE);
    fprintf(f, "%s", "invalid_key");
    fclose(f);

    assert(run(NULL, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(DEFAULT_DATA_STORE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_KEY_FILE_FLAG_DATA) != 0, t);

    free(decoded_data);

    remove(TEST_KEY_FILE);
    remove(DEFAULT_DATA_STORE);
}

void test_input_flag_empty(Test *t)
{
    test(run(t->a.key, t->a.argc, t->a.argv) == 1, t);
}

void test_input_flag_non_existent_file(Test *t)
{
    test(run_test_in_fork(&t->a) == 1, t);
}

void test_input_flag_write_data(Test *t)
{
    assert(run(t->a.key, t->a.argc, t->a.argv) == 0);

    size_t nch = 0;
    char  *data = read_file_as_str_test(TEST_DATA_FILE, &nch);

    size_t         decsize = 0;
    unsigned char *decoded_data = b64_decode_ex(data, nch, &decsize);

    AES_init_ctx_iv(&ctx, t->a.key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

    test(strcmp((char *)decoded_data, TEST_INPUT_FLAG_DATA) == 0, t);

    free(decoded_data);

    remove(TEST_DATA_FILE);
}

void run_test(Test *t) { t->f(t); }

int main(void)
{
    Test tests[] = {
        {
            .a = {.argc = 1, .argv = calloc(1, 256)},
            .c = {.t = NO_FLAG, .n = NO_FLAG_CATEGORY_NAME},
            .d = ".pm_data exists",
            .f = test_no_flag_pm_data_exists,
        },
        {
            .a = {.argc = 1, .argv = calloc(1, 256)},
            .c = {.t = NO_FLAG, .n = NO_FLAG_CATEGORY_NAME},
            .d = ".pm_data doesn't exist",
            .f = test_no_flag_pm_data_doesnt_exist,
        },
        {
            .a = ARGS("-d"),
            .c = {.t = DATA, .n = DATA_CATEGORY_NAME},
            .d = "empty",
            .f = test_data_flag_empty,
        },
        {
            .a = ARGS("-d", "data"),
            .c = {.t = DATA, .n = DATA_CATEGORY_NAME},
            .d = "valid",
            .f = test_data_flag_valid,
        },
        {
            .a = ARGS("-d", "data", "-gp"),
            .c = {.t = DATA, .n = DATA_CATEGORY_NAME},
            .d = "with generate password",
            .f = test_data_flag_with_generate_password,
        },
        {
            .a = ARGS("-d", "data", "-df", TEST_DATA_FILE),
            .c = {.t = DATA, .n = DATA_CATEGORY_NAME},
            .d = "with data file",
            .f = test_data_flag_with_data_file,
        },
        {
            .a = ARGS("-df"),
            .c = {.t = DATA_FILE, .n = DATA_FILE_CATEGORY_NAME},
            .d = "empty",
            .f = test_data_file_flag_empty,
        },
        {
            .a = ARGS("-df", TEST_DATA_FILE),
            .c = {.t = DATA_FILE, .n = DATA_FILE_CATEGORY_NAME},
            .d = "empty file",
            .f = test_data_file_flag_empty_file,
        },
        {
            .a = ARGS("-df", TEST_DATA_FILE),
            .c = {.t = DATA_FILE, .n = DATA_FILE_CATEGORY_NAME},
            .d = "non-existent",
            .f = test_data_file_flag_non_existent_file,
        },
        {
            .a = ARGS("-df", TEST_DATA_FILE),
            .c = {.t = DATA_FILE, .n = DATA_FILE_CATEGORY_NAME},
            .d = "valid",
            .f = test_data_file_flag_valid,
        },
        {
            .a = ARGS("-df", TEST_DATA_FILE, "-gp"),
            .c = {.t = DATA_FILE, .n = DATA_FILE_CATEGORY_NAME},
            .d = "with generate password",
            .f = test_data_file_flag_with_generate_password,
        },
        {
            .a = ARGS("-l"),
            .c = {.t = LABEL, .n = LABEL_CATEGORY_NAME},
            .d = "empty",
            .f = test_label_flag_empty,
        },
        {
            .a = ARGS("-l", TEST_LABEL_FLAG_LABEL, "-d", TEST_LABEL_FLAG_DATA),
            .c = {.t = LABEL, .n = LABEL_CATEGORY_NAME},
            .d = "valid",
            .f = test_label_flag_valid,
        },
        {
            .a = ARGS("-l", TEST_LABEL_FLAG_LABEL, "-d",
                      TEST_LABEL_FLAG_REPLACE_DATA),
            .c = {.t = LABEL, .n = LABEL_CATEGORY_NAME},
            .d = "replace",
            .f = test_label_flag_replace,
        },
        {
            .a = ARGS("-dl", "label_that_doesnt_exist"),
            .c = {.t = DELETE_LABEL, .n = DELETE_LABEL_CATEGORY_NAME},
            .d = "doesn't exist",
            .f = test_delete_label_flag_doesnt_exist,
        },
        {
            .a = ARGS("-dl", TEST_LABEL_FLAG_LABEL),
            .c = {.t = DELETE_LABEL, .n = DELETE_LABEL_CATEGORY_NAME},
            .d = "exists",
            .f = test_delete_label_flag_exists,
        },
        {
            .a = ARGS("-fl"),
            .c = {.t = FIND_LABEL, .n = FIND_LABEL_CATEGORY_NAME},
            .d = "empty",
            .f = test_find_label_flag_empty,
        },
        {
            .a = ARGS("-dl"),
            .c = {.t = DELETE_LABEL, .n = DELETE_LABEL_CATEGORY_NAME},
            .d = "empty",
            .f = test_delete_label_flag_empty,
        },
        {
            .a = ARGS("-gp"),
            .c = {.t = GENERATE_PASSWORD, .n = GENERATE_PASSWORD_CATEGORY_NAME},
            .d = "empty",
            .f = test_generate_password_flag_empty,
        },
        {
            .a = ARGS("-gp", "128"),
            .c = {.t = GENERATE_PASSWORD, .n = GENERATE_PASSWORD_CATEGORY_NAME},
            .d = "128 chars",
            .f = test_generate_password_flag_128_chars,
        },
        {
            .a = ARGS("-k"),
            .c = {.t = KEY, .n = KEY_CATEGORY_NAME},
            .d = "empty",
            .f = test_key_flag_empty,
        },
        {
            .a = ARGS("-k", AES_KEY, "-d", TEST_KEY_FLAG_DATA),
            .c = {.t = KEY, .n = KEY_CATEGORY_NAME},
            .d = "valid",
            .f = test_key_flag_valid,
        },
        {
            .a = ARGS("-k", "invalid_key", "-d", TEST_KEY_FLAG_DATA),
            .c = {.t = KEY, .n = KEY_CATEGORY_NAME},
            .d = "invalid",
            .f = test_key_flag_invalid,
        },
        {
            .a = ARGS("-kf"),
            .c = {.t = KEY_FILE, .n = KEY_FILE_CATEGORY_NAME},
            .d = "empty",
            .f = test_key_file_flag_empty,
        },
        {
            .a = ARGS("-kf", "keyfile_that_doesnt_exist.txt"),
            .c = {.t = KEY_FILE, .n = KEY_FILE_CATEGORY_NAME},
            .d = "non-existent",
            .f = test_key_file_flag_non_existent_file,
        },
        {
            .a = ARGS("-kf", TEST_KEY_FILE, "-d", TEST_KEY_FILE_FLAG_DATA),
            .c = {.t = KEY_FILE, .n = KEY_FILE_CATEGORY_NAME},
            .d = "valid",
            .f = test_key_file_flag_valid,
        },
        {
            .a = ARGS("-kf", TEST_KEY_FILE, "-d", TEST_KEY_FILE_FLAG_DATA),
            .c = {.t = KEY_FILE, .n = KEY_FILE_CATEGORY_NAME},
            .d = "invalid",
            .f = test_key_file_flag_invalid,
        },
        {
            .a = ARGS("-i"),
            .c = {.t = INPUT_, .n = INPUT_CATEGORY_NAME},
            .d = "empty",
            .f = test_input_flag_empty,
        },
        {
            .a = ARGS("-i", "input_file_that_doesnt_exist.txt"),
            .c = {.t = INPUT_, .n = INPUT_CATEGORY_NAME},
            .d = "non-existent",
            .f = test_input_flag_non_existent_file,
        },
        {
            .a = ARGS("-i", TEST_DATA_FILE, "-d", TEST_INPUT_FLAG_DATA),
            .c = {.t = INPUT_, .n = INPUT_CATEGORY_NAME},
            .d = "write data",
            .f = test_input_flag_write_data,
        },
    };

    size_t tests_count = sizeof(tests) / sizeof(tests[0]);

    uint8_t *key = calloc(1, 128);
    memcpy(key, AES_KEY, sizeof(AES_KEY));

    for (size_t i = 0; i < tests_count; i++)
    {
        tests[i].a.key = key;
        run_test(&tests[i]);
        free_argv(&tests[i].a);
    }

    printf("\n(%zu / %zu) tests passed\n", tests_count - failed_tests_count,
           tests_count);

    free(key);
    return failed_tests_count >= 1;
}
