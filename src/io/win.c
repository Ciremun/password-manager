#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#include <windows.h>

void exit_program(int exit_code)
{
    ExitThread(exit_code);
}

size_t getpasswd(char **pw)
{
    int c;
    int idx = 0;
    size_t buf = 128;

    if (*pw == NULL)
    {
        *pw = calloc(1, buf);
    }

    while ((c = _getch()) != 13 && c != EOF)
    {
        if (c != 127)
        {
            if (idx >= buf)
            {
                buf *= 2;
                *pw = realloc(*pw, buf);
            }
            (*pw)[idx++] = c;
        }
        else if (idx > 0)
        {
            (*pw)[--idx] = 0;
        }
    }

    if (buf != 128)
    {
        *pw = realloc(*pw, idx);
    }

    return idx;
}