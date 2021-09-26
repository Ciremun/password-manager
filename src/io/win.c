#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#include <windows.h>

size_t getpasswd(char **pw)
{
    int c;
    size_t idx = 0;
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

int copy_to_clipboard(const char* password, size_t size)
{
    if (!OpenClipboard(0)) {
        return 0;
    }
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    memcpy(GlobalLock(hMem), password, size);
    GlobalUnlock(hMem);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    GlobalFree(hMem);
    return 1;
}
