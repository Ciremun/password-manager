#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "../src/io/common.h"

#define TABS "\t\t\t"
#define AES_KEY "test_aes_key!@#$%^&*();'"

typedef struct 
{
    int argc;
    char **argv;
} Args;

uint8_t *get_key(void);
int run_test_in_fork(Args *a);
char **fill_args(int argc, ...);
void assert_t(int check, const char *test);
void free_argv(int argc, char **argv);
void setup_test(void);
void exit_test(void);
void run_test(void (*test)(void));
void test_no_flag(void);
void test_data_flag(void);
void test_data_file_flag(void);
void test_label_flag(void);

void test_generate_password_flag(void);
void test_key_flag(void);
