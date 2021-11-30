#ifndef PM_IO_H_
#define PM_IO_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <windows.h>
typedef HANDLE handle_t;
#define PM_BACKSPACE_KEY 8
#define PM_BAD_FILE_HANDLE INVALID_HANDLE_VALUE
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
typedef int handle_t;
#define PM_BACKSPACE_KEY 127
#define PM_BAD_FILE_HANDLE -1
#endif // _WIN32

#include "pm_parse.h"

#ifdef TEST
void exit_test_case(int exit_code);
#define exit exit_test_case
#define error(fmt, ...)
#define info(fmt, ...)
#else
#define error(fmt, ...) fprintf(stderr, "error: " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) fprintf(stdout, "info: " fmt "\n", __VA_ARGS__)
#endif // TEST

#define DEFAULT_DATA_STORE ".pm_data"

#define MAP_FILE_OR_EXIT(file_ptr) \
    do                             \
    {                              \
        if (!map_file(file_ptr))   \
            exit(1);               \
    } while (0)

#define TRUNCATE_FILE_OR_EXIT(h, new_size) \
    do                                     \
    {                                      \
        if (!truncate_file(h, new_size))   \
            exit(1);                       \
    } while (0)

#define EXIT_IF_BAD_FILE_HANDLE(handle)   \
    do                                    \
    {                                     \
        if (handle == PM_BAD_FILE_HANDLE) \
            exit(1);                      \
    } while (0)

typedef enum
{
    PM_READ_WRITE = 0,
    PM_READ_ONLY,
} flag_t;

typedef struct
{
#ifdef _WIN32
    HANDLE hMap;
#endif // _WIN32
    handle_t handle;
    flag_t access;
    size_t size;
    uint8_t *start;
} File;

File open_or_create_file(const char *path, flag_t access, int create);
File open_file(const char *path, flag_t access);
File create_file(const char *path, flag_t access);
File open_and_map_file(const char *path, flag_t access);
File create_and_map_file(const char *path, flag_t access);
int close_file(handle_t handle);
int file_exists(const char *path);
int truncate_file(File *f, size_t new_size);
int get_file_size(File *f);
int map_file(File *f);
int unmap_file(File f);
int unmap_and_close_file(File f);
int getpasswd(uint8_t *pw);
void input_key(uint8_t *aes_key, Flags *f);

#endif // PM_IO_H_
