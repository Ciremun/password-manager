// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pm_event.h"

volatile int suspended;
extern InputField inputs[1];

void HandleKey(int keycode, int bDown)
{
    if (bDown)
    {
        inputs[0].oninput(&inputs[0], keycode);
    }
}

void HandleButton(int x, int y, int button, int bDown)
{
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
