#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "../src/io/common.h"

#define TABS "\t\t"

typedef struct 
{
    int argc;
    char **argv;
} Args;

void exit_tests(void);
void assert_t(int check, const char *test);
char **fill_args(int argc, ...);
void free_argv(int argc, char **argv);
void setup_test(void);
void exit_test(void);
uint8_t *get_key(void);
void reset_key();
void run_test(void (*test)(void));
void test_no_flag(void);
void test_data_flag(void);
void test_data_file_flag(void);
void test_label_flag(void);

void test_generate_password_flag(void);
