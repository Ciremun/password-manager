// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pm_event.h"

volatile int suspended;

int offset = 0;
char str[64] = {0};

void HandleKey(int keycode, int bDown)
{
    if (bDown)
        str[offset++] = keycode;
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
