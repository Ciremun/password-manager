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

#include "sync.h"

#ifndef _WIN32
Cstr_Array cstr_array_append(Cstr_Array cstrs, Cstr cstr)
{
    Cstr_Array result = {.count = cstrs.count + 1};
    result.elems = (Cstr *)alloc(sizeof(result.elems[0]) * result.count);
    memcpy(result.elems, cstrs.elems, cstrs.count * sizeof(result.elems[0]));
    result.elems[cstrs.count] = cstr;
    return result;
}
#endif // _WIN32

int verify_remote(const char *remote)
{
    if (remote == NULL)
        return 0;
    const char *git_config = ".git/config";
    FILE       *f = fopen(git_config, "rb");
    if (f == NULL)
    {
        error("opening file %s\n", git_config);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char  *str = (char *)alloc(size + 1);
    if (str == NULL)
    {
        error("%s:%d memory allocation failed\n", __FILE__, __LINE__);
        fclose(f);
        return 0;
    }
    fseek(f, 0, SEEK_SET);
    fread(str, 1, size, f);
    str[size] = '\0';
    fclose(f);
    for (size_t i = 0; i + 5 < size; ++i)
    {
        if (str[i + 0] == 'u' && str[i + 1] == 'r' && str[i + 2] == 'l'
            && str[i + 3] == ' ' && str[i + 4] == '=' && str[i + 5] == ' ')
        {
            i += 6;
            size_t start = i;
            for (; i < size; ++i)
            {
                if (str[i] == '\n' || str[i] == '\r')
                {
                    if (memcmp(str + start, remote, i - start) == 0)
                    {
                        return 1;
                    }
                }
            }
            break;
        }
    }
    error("provided remote (%s) doesn't match origin in .git/config\n", remote);
    return 0;
}

int pull_changes(const char *remote)
{
    if (!verify_remote(remote))
        return -1;
    CMD("git", "checkout", "master", "--quiet");
    CMD("git", "pull", remote, "master", "--quiet");
    return 0;
}

int upload_changes(const char *remote)
{
    if (!verify_remote(remote))
        return -1;
    CMD("git", "add", "-A");
    CMD("git", "commit", "-m", "auto_upload", "--quiet");
    CMD("git", "push", remote, "master", "--quiet");
    return 0;
}

Cstr_Array cstr_array_make(Cstr first, ...)
{
    Cstr_Array result = {0};

    if (first == NULL)
    {
        return result;
    }

    result.count += 1;

    va_list args;
    va_start(args, first);
    for (Cstr next = va_arg(args, Cstr); next != NULL;
         next = va_arg(args, Cstr))
    {
        result.count += 1;
    }
    va_end(args);

    result.elems = (Cstr *)alloc(sizeof(result.elems[0]) * result.count);
    if (result.elems == NULL)
    {
        PANIC("could not allocate memory: %s", strerror(errno));
    }
    result.count = 0;

    result.elems[result.count++] = first;

    va_start(args, first);
    for (Cstr next = va_arg(args, Cstr); next != NULL;
         next = va_arg(args, Cstr))
    {
        result.elems[result.count++] = next;
    }
    va_end(args);

    return result;
}

Cstr cstr_array_join(Cstr sep, Cstr_Array cstrs)
{
    if (cstrs.count == 0)
    {
        return "";
    }

    const size_t sep_len = strlen(sep);
    size_t       len = 0;
    for (size_t i = 0; i < cstrs.count; ++i)
    {
        len += strlen(cstrs.elems[i]);
    }

    const size_t result_len = (cstrs.count - 1) * sep_len + len + 1;
    char        *result = (char *)alloc(sizeof(char) * result_len);
    if (result == NULL)
    {
        PANIC("could not allocate memory: %s", strerror(errno));
    }

    len = 0;
    for (size_t i = 0; i < cstrs.count; ++i)
    {
        if (i > 0)
        {
            memcpy(result + len, sep, sep_len);
            len += sep_len;
        }

        size_t elem_len = strlen(cstrs.elems[i]);
        memcpy(result + len, cstrs.elems[i], elem_len);
        len += elem_len;
    }
    result[len] = '\0';

    return result;
}

Cstr cmd_show(Cmd cmd) { return cstr_array_join(" ", cmd.line); }

int pid_wait(Pid pid)
{
#ifdef _WIN32
    DWORD result = WaitForSingleObject(pid,     // HANDLE hHandle,
                                       INFINITE // DWORD  dwMilliseconds
    );

    if (result == WAIT_FAILED)
    {
        PANIC("could not wait on child process: %lu", GetLastError());
    }

    DWORD exit_status;
    if (GetExitCodeProcess(pid, &exit_status) == 0)
    {
        PANIC("could not get process exit code: %lu", GetLastError());
    }

    CloseHandle(pid);

    return (int)exit_status;
#else
    for (;;)
    {
        int wstatus = 0;
        if (waitpid(pid, &wstatus, 0) < 0)
        {
            PANIC("could not wait on command (pid %d): %s", pid,
                  strerror(errno));
        }

        if (WIFEXITED(wstatus))
        {
            return WEXITSTATUS(wstatus);
        }

        if (WIFSIGNALED(wstatus))
        {
            PANIC("command process was terminated by %s",
                  strsignal(WTERMSIG(wstatus)));
        }
    }

#endif // _WIN32
}

Pid cmd_run_async(Cmd cmd, Fd *fdin, Fd *fdout)
{
#ifdef _WIN32
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = fdout ? *fdout : GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = fdin ? *fdin : GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    char *lpCommandLine = (char *)cstr_array_join(" ", cmd.line);
    BOOL  bSuccess = CreateProcess(NULL, lpCommandLine, NULL, NULL, TRUE, 0,
                                   NULL, NULL, &siStartInfo, &piProcInfo);

    if (!bSuccess)
    {
        PANIC("Could not create child process %s: %lu\n", cmd_show(cmd),
              GetLastError());
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if (cpid < 0)
    {
        PANIC("Could not fork child process: %s: %s", cmd_show(cmd),
              strerror(errno));
    }

    if (cpid == 0)
    {
        Cstr_Array args = cstr_array_append(cmd.line, NULL);

        if (fdin)
        {
            if (dup2(*fdin, STDIN_FILENO) < 0)
            {
                PANIC("Could not setup stdin for child process: %s",
                      strerror(errno));
            }
        }

        if (fdout)
        {
            if (dup2(*fdout, STDOUT_FILENO) < 0)
            {
                PANIC("Could not setup stdout for child process: %s",
                      strerror(errno));
            }
        }

        if (execvp(args.elems[0], (char *const *)args.elems) < 0)
        {
            PANIC("Could not exec child process: %s: %s", cmd_show(cmd),
                  strerror(errno));
        }
    }

    return cpid;
#endif // _WIN32
}

int cmd_run_sync(Cmd cmd) { return pid_wait(cmd_run_async(cmd, NULL, NULL)); }
