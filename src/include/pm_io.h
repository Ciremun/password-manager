#ifndef PM_IO_H_
#define PM_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
typedef HANDLE handle_t;
#define PM_BACKSPACE_KEY 8
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
typedef int handle_t;
#define PM_BACKSPACE_KEY 127
#endif // _WIN32

#include <stdint.h>

#include "pm_parse.h"

#ifdef _WIN32
#define PM_BAD_FILE_HANDLE INVALID_HANDLE_VALUE
#else
#define PM_BAD_FILE_HANDLE -1
#endif // _WIN32

#define DEFAULT_DATA_STORE ".pm_data"

typedef enum
{
    PM_READ_ONLY = 0,
    PM_READ_WRITE
} flag_t;

typedef struct
{
#ifdef _WIN32
    HANDLE hMap;
#endif // _WIN32
    handle_t handle;
    flag_t access;
    size_t size;
} File;

File open_or_create_file(const char *path, flag_t access, int create);
int close_file(File f);
int file_exists(const char *path);
int truncate_file(handle_t h, size_t new_size);
int get_file_size(File *f);
char *map_file(File f);
int unmap_file(char *map_start, size_t size);
int getpasswd(uint8_t *pw);
void input_key(uint8_t *aes_key, Flags *f);

#endif // PM_IO_H_
