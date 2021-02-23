#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#include "io_win.h"

void getpasswd(char **pw, int sz)
{
    int c;
    int idx = 0;

    if (*pw == NULL)
    {
        void *tmp = (void *)realloc(*pw, sz * sizeof **pw);
        if (!tmp)
            return;
        memset(tmp, 0, sz);
        *pw = (char *)tmp;
    }

    while (((c = _getch()) != 13 && c != EOF && idx < sz - 1) || (idx == sz - 1 && c == 127))
    {
        if (c != 127)
        {
            (*pw)[idx++] = c;
        }
        else if (idx > 0)
        {
            (*pw)[--idx] = 0;
        }
    }
}