#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include "win.h"
#else
#include "unix.h"
#endif

int main()
{
    system("pm.exe -d 123");
    system("123");
}
