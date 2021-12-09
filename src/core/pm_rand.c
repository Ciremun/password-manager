#include "pm_rand.h"

#include <time.h>
#include <stdlib.h>

int random_int()
{
    srand((unsigned int)time(NULL));
    rand();
    int x = 33;
    while (x > 32)
        x = 16 + rand() / ((RAND_MAX + 1u) / 32);
    return x;
}

void random_string(size_t sz, uint8_t *out)
{
    srand((unsigned int)time(NULL));
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX"
                            "YZ0123456789!@#$%^&*()-+";
    size_t l = sizeof(charset) - 1;
    for (size_t n = 0; n < sz; n++)
        out[n] = charset[rand() % l];
    out[sz] = '\0';
}
