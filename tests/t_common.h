#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "../src/io/common.h"

#define TABS "\t"

void exit_tests(void);
void assert_t(int check, const char *test);
char **fill_args(int argc, ...);
void free_argv(int argc, char **argv);
void setup_test(void);
void exit_test(void);
void reset_key();
void run_test(void (*test)(void));
void test_data_flag(void);
void test_no_flag(void);

extern uint8_t *aes_key;
