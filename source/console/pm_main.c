// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#ifdef _MSC_VER
#pragma comment(lib, "User32")
#endif // _MSC_VER

#include <stddef.h>
#include <stdint.h>

#include "console/io.h"
#include "console/parse.h"
#include "console/sync.h"

#include <stdio.h>

uint8_t aes_key[32] = {0};

int main(int argc, char **argv)
{
    return run(aes_key, argc, argv);
}
