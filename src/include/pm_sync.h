#ifndef PM_SYNC_H_
#define PM_SYNC_H_

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

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "pm_util.h"

typedef const char *Cstr;

typedef struct
{
    Cstr *elems;
    size_t count;
} Cstr_Array;

typedef struct
{
    Cstr_Array line;
} Cmd;

#ifdef _WIN32
LPSTR GetLastErrorAsString(void);
#endif // _WIN32

#define return_code_if_error(code) \
    if (code != 0)                 \
    return code

#define CMD(...)                                                \
    do                                                          \
    {                                                           \
        Cmd cmd = {.line = cstr_array_make(__VA_ARGS__, NULL)}; \
        return_code_if_error(cmd_run_sync(cmd));                \
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

#endif // PM_SYNC_H_