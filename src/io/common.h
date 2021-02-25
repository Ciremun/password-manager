#ifndef IO_COMMON_H
#define IO_COMMON_H

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define LMAX 255


ssize_t getline(char **lineptr, size_t *n, FILE *stream);
void read_file(const char *fp, char ***lines, size_t *lsize);
char *read_file_as_str(const char *fp, size_t *nch);
void write_file(const char *fp, const char *mode, void *data);

#endif // IO_COMMON_H
