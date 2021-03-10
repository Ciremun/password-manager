#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "unix.h"

void exit_program(int exit_code)
{
    exit(exit_code);
}

ssize_t getpasswd(char **pw)
{
    FILE *fp = stdin;
    size_t buf = 128;

    if (*pw == NULL)
    {
        *pw = calloc(1, buf);
    }

    size_t idx = 0;
    int c = 0;

    struct termios old_kbd_mode;
    struct termios new_kbd_mode;

    if (tcgetattr(0, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcgetattr failed\n", __func__);
        return -1;
    }
    memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr(0, TCSANOW, &new_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed\n", __func__);
        return -1;
    }

    while ((c = fgetc(fp)) != '\n' && c != EOF)
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
    (*pw)[idx] = 0;
    if (buf != 128)
    {
        *pw = realloc(*pw, idx);
    }
    if (tcsetattr(0, TCSANOW, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed\n", __func__);
        return -1;
    }

    return idx;
}
