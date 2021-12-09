// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <stddef.h>
#include <stdint.h>

#include "pm_io.h"
#include "pm_parse.h"
#include "pm_sync.h"

#include <stdio.h>

uint8_t aes_key[32] = {0};

int main(int argc, char **argv)
{
    return run(aes_key, argc, argv);
}
