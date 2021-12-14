// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <stddef.h>

#include "rawdraw/rd_event.h"
#include "rawdraw/rd_util.h"

volatile int suspended;
extern InputFields input_fields;

void oninput(InputField *i, int keycode)
{
    if (!i->focused)
        return;
    if (keycode == BACKSPACE_KEY)
    {
        if (i->text.offset)
            i->text.string.data[--i->text.offset] = 0;
    }
    else
        i->text.string.data[i->text.offset++] = keycode;
}

void HandleKey(int keycode, int bDown)
{
    if (bDown)
    {
        for (size_t i = 0; i < input_fields.count; ++i)
            input_fields.arr[i].oninput(&input_fields.arr[i], keycode);
    }
}

void HandleButton(int x, int y, int button, int bDown)
{
    if (bDown)
    {
        Point click = {.x = x, .y = y};
        for (size_t i = 0; i < input_fields.count; ++i)
        {
            input_fields.arr[i].focused = inside_rect(click, input_fields.arr[i].rect);
            ;
            if (input_fields.arr[i].focused)
            {
                ++i;
                for (; i < input_fields.count; ++i)
                    input_fields.arr[i].focused = 0;
                return;
            }
        }
    }
}

void HandleMotion(int x, int y, int mask)
{
}

void HandleDestroy()
{
}

void HandleSuspend()
{
    suspended = 1;
}
void HandleResume()
{
    suspended = 0;
}
