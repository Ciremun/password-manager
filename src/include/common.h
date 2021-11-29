#ifndef IO_COMMON_H
#define IO_COMMON_H

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
#include "pm_io.h"
#include "parse.h"

#define DEFAULT_DATA_STORE ".pm_data"

#ifdef TEST
void exit_test_case(int exit_code);
#define exit exit_test_case
#define error(fmt, ...)
#define info(fmt, ...)
#else
#define error(fmt, ...) fprintf(stderr, "error: " fmt, __VA_ARGS__)
#define info(fmt, ...) fprintf(stdout, "info: " fmt, __VA_ARGS__)
#endif // TEST

#ifdef _WIN32
int copy_to_clipboard(const char *password, size_t size);
#endif

int getpasswd(char **pw);
unsigned char *decrypt_base64(const char *line, uint8_t *aes_key,
                              size_t line_length, size_t *decoded_line_length);
void decrypt_raw(uint8_t *line, uint8_t *aes_key, size_t length);

void decrypt_and_print(uint8_t *aes_key, Flags *f);
void encrypt_and_write(Flags *f, uint8_t *data, uint8_t *aes_key,
                       size_t data_length);
void encrypt_and_replace(Flags *f, char *find_label, char *data,
                         uint8_t *aes_key, size_t data_length);
void delete_label(char *label, uint8_t *aes_key);

#endif // IO_COMMON_H
