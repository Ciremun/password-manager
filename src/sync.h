// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#ifndef SYNC_H
#define SYNC_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "io/common.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE Pid;
typedef HANDLE Fd;
#else
#include <sys/wait.h>
#include <unistd.h>
typedef pid_t Pid;
typedef int Fd;
#endif // _WIN32

typedef char* Cstr;

typedef struct {
    Cstr *elems;
    size_t count;
} Cstr_Array;

typedef struct {
    Cstr_Array line;
} Cmd;

#ifdef _WIN32
LPSTR GetLastErrorAsString(void);
#endif // _WIN32

#define return_code_if_error(code) if (code != 0) return code

#define CMD(...)                                        \
    do {                                                \
        Cmd cmd = {                                     \
            .line = cstr_array_make(__VA_ARGS__, NULL)  \
        };                                              \
        return_code_if_error(cmd_run_sync(cmd));        \
        free(cmd.line.elems);                           \
    } while (0)

int verify_remote(const char *remote);
int pull_changes(const char *remote);
int upload_changes(const char *remote);
int pid_wait(Pid pid);
int cmd_run_sync(Cmd cmd);
Pid cmd_run_async(Cmd cmd, Fd *fdin, Fd *fdout);
Cstr cmd_show(Cmd cmd);
Cstr cstr_array_join(Cstr sep, Cstr_Array cstrs);
Cstr_Array cstr_array_make(Cstr first, ...);
Cstr_Array cstr_array_append(Cstr_Array cstrs, Cstr cstr);

#endif // SYNC_H
