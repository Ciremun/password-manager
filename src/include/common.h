#ifndef IO_COMMON_H
#define IO_COMMON_H

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#endif // _WIN32

#include "aes.h"
#include "b64.h"
#include "mem.h"
#include "parse.h"

#define LMAX               255
#define DEFAULT_DATA_STORE ".pm_data"

#ifdef TEST
void exit_test_case(int exit_code);
#define exit exit_test_case
#define error(fmt, ...)
#define info(fmt, ...)
#else
#define error(fmt, ...) fprintf(stderr, "error: " fmt, __VA_ARGS__)
#define info(fmt, ...)  fprintf(stdout, "info: " fmt, __VA_ARGS__)
#endif // TEST

#define PANIC(FMT, ...)                                                        \
    do                                                                         \
    {                                                                          \
        error(FMT, __VA_ARGS__);                                               \
        exit(1);                                                               \
    } while (0)

#define PANIC_OPEN_FILE(PATH) PANIC("opening file %s\n", PATH)

typedef struct
{
    char  *data;
    size_t length;
} Line;

typedef struct
{
    Line  *array;
    size_t count;
} Lines;

#ifdef _WIN32
int copy_to_clipboard(const char *password, size_t size);
#endif

int            getpasswd(char **pw);
ssize_t        getline(char **lineptr, size_t *n, FILE *stream);
void           read_file(const char *fp, char ***lines, size_t *lsize);
char          *read_file_as_str(const char *fp, size_t *nch);
unsigned char *decode_line(const char *line, uint8_t *aes_key,
                           size_t line_length, size_t *decoded_line_length);
void           write_file(const char *fp, const char *mode, void *data, size_t size);
Lines          decrypt_and_find(uint8_t *aes_key, Flags *f);
void           decrypt_and_print(uint8_t *aes_key, Flags *f);
void           encrypt_and_write(Flags *f, uint8_t *data, uint8_t *aes_key,
                                 size_t data_length);
void           encrypt_and_replace(Flags *f, char *find_label, char *data,
                                   uint8_t *aes_key);
void           delete_label(char *label, uint8_t *aes_key);
void          *alloc(u64 size);

#endif // IO_COMMON_H
