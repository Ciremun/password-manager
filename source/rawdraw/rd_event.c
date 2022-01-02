// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <stddef.h>

#include "rawdraw/rd_event.h"
#include "rawdraw/rd_util.h"

volatile int suspended;

extern short h;
extern InputFields input_fields;
extern Keyboard keyboard;
extern RDWindow window;

void oninput(InputField *i, int keycode)
{
    if (!i->focused)
        return;
    if (keycode == BACKSPACE_KEY)
    {
        if (i->text.string.length)
            i->text.string.data[--i->text.string.length] = 0;
    }
    else
        i->text.string.data[i->text.string.length++] = keycode;
}

void HandleKey(int keycode, int bDown)
{
    printf("HandleKey: %d %d\n", keycode, bDown);
    if (bDown && keycode == CAPSLOCK_KEY)
    {
        keyboard.uppercase = !keyboard.uppercase;
        return;
    }
    if (keycode == SHIFT_KEY)
    {
        keyboard.uppercase = bDown;
        return;
    }
    if (bDown)
    {
        if (keyboard.uppercase && is_alpha(keycode))
            keycode -= 32;
        for (size_t i = 0; i < input_fields.count; ++i)
            input_fields.arr[i].oninput(&input_fields.arr[i], keycode);
    }
}

void HandleButton(int x, int y, int button, int bDown)
{
    printf("HandleButton: %d %d %d %d\n", x, y, button, bDown);
    if (bDown)
    {
        if (button == MOUSE_SCROLL_UP)
        {
            if (window.scroll)
            {
                int offset = RD_INPUT_FIELD_HEIGHT + RD_INPUT_FIELD_MARGIN;
                window.scroll--;
                for (size_t i = window.scroll; i < input_fields.count; ++i)
                {
                    input_fields.arr[i].rect.p1.y += offset;
                    input_fields.arr[i].rect.p2.y += offset;
                }
                HandleMotion(x, y, 0);
            }
            return;
        }
        if (button == MOUSE_SCROLL_DOWN)
        {
            int offset = RD_INPUT_FIELD_HEIGHT + RD_INPUT_FIELD_MARGIN;
            if (window.scroll + 1 < input_fields.count)
            {
                for (size_t i = window.scroll; i < input_fields.count; ++i)
                {
                    input_fields.arr[i].rect.p1.y -= offset;
                    input_fields.arr[i].rect.p2.y -= offset;
                }
                window.scroll++;
                HandleMotion(x, y, 0);
            }
            return;
        }
        Point click = {.x = x, .y = y};
        for (size_t i = 0; i < input_fields.count; ++i)
        {
            input_fields.arr[i].focused = inside_rect(click, input_fields.arr[i].rect);
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
    // printf("HandleMotion: %d %d %d\n", x, y, mask);
    Point hover = {.x = x, .y = y};
    for (size_t i = 0; i < input_fields.count; ++i)
    {
        input_fields.arr[i].rect.color = inside_rect(hover, input_fields.arr[i].rect) ? SILVER : WHITE;
        if (input_fields.arr[i].rect.color == SILVER)
        {
            ++i;
            for (; i < input_fields.count; ++i)
                input_fields.arr[i].rect.color = WHITE;
            return;
        }
    }
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
