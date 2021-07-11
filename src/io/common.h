#ifndef IO_COMMON_H
#define IO_COMMON_H

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../b64/b64.h"
#include "../aes.h"

#include "../parse.h"

#define LMAX 255
#define DATA_STORE ".data"

#ifdef _WIN32
int copy_to_clipboard(const char *password, size_t size);
#endif

size_t getpasswd(char **pw);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
void read_file(const char *fp, char ***lines, size_t *lsize);
char *read_file_as_str(const char *fp, size_t *nch);
unsigned char *decode_line(const char *line, uint8_t *aes_key, size_t *decoded_line_length);
void write_file(const char *fp, const char *mode, void *data);
void decrypt_and_print(uint8_t *aes_key, Flags *f);
void encrypt_and_write(uint8_t *data, uint8_t *aes_key, size_t data_length);
void encrypt_and_replace(char *find_label, char *data, uint8_t *aes_key);
void delete_label(char *label, uint8_t *aes_key);
void exit_program(int exit_code);

#define PANIC(FMT, ...)                    \
    do                                     \
    {                                      \
        fprintf(stderr, FMT, __VA_ARGS__); \
        exit_program(1);                   \
    } while (0)

#define PANIC_OPEN_FILE(PATH) PANIC("error opening file %s\n", PATH)
#define PANIC_MALLOC() PANIC("%s:%d memory allocation failed\n", __FILE__, __LINE__)

#endif // IO_COMMON_H
