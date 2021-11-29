#include <stddef.h>
#include <stdint.h>

#include "pm_parse.h"

int main(int argc, char **argv)
{
    uint8_t *aes_key = NULL;
    return run(aes_key, argc, argv);
}
