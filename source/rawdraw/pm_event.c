// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pm_event.h"
#include "pm_util.h"

volatile int suspended;
extern InputField inputs[1];

void HandleKey(int keycode, int bDown)
{
    if (bDown)
    {
        for (int i = 0; i < 2; ++i)
            inputs[i].oninput(&inputs[i], keycode);
    }
}

void HandleButton(int x, int y, int button, int bDown)
{
    if (bDown)
    {
        Point click = (Point){ .x = x, .y = y };
        for (int i = 0; i < 2; ++i)
        {
            inputs[i].focused = inside_rect(click, inputs[i].rect);;
            if (inputs[i].focused)
            {
                for (int j = i + 1; j < 2; ++j)
                    inputs[j].focused = 0;
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
