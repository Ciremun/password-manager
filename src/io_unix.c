#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "io_unix.h"

ssize_t getpasswd(char **pw, size_t sz)
{
    if (!pw || !sz)
        return -1;

    FILE *fp = stdin;

    if (*pw == NULL)
    {
        void *tmp = realloc(*pw, sz * sizeof **pw);
        if (!tmp)
            return -1;
        memset(tmp, 0, sz);
        *pw = (char *)tmp;
    }

    size_t idx = 0;
    int c = 0;

    struct termios old_kbd_mode;
    struct termios new_kbd_mode;

    if (tcgetattr(0, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcgetattr failed.\n", __func__);
        return -1;
    }
    memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr(0, TCSANOW, &new_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    while (((c = fgetc(fp)) != '\n' && c != EOF && idx < sz - 1) ||
           (idx == sz - 1 && c == 127))
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
    (*pw)[idx] = 0;

    if (tcsetattr(0, TCSANOW, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    if (idx == sz - 1 && c != '\n')
        fprintf(stderr, " (%s() warning: truncated at %zu chars.)\n",
                __func__, sz - 1);

    return idx;
}
