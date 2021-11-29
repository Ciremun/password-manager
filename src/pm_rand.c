#include "pm_rand.h"

#include <stdlib.h>

int random_int()
{
    srand(time(NULL));
    rand();
    int x = 33;
    while (x > 32)
        x = 16 + rand() / ((RAND_MAX + 1u) / 32);
    return x;
}

void random_string(int sz, char *out)
{
    srand(time(NULL));
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWX"
                            "YZ0123456789!@#$%^&*()-+";
    int l = (int)(sizeof(charset) - 1);
    for (int n = 0; n < sz; n++)
    {
        out[n] = charset[rand() % l];
    }
    out[sz] = '\0';
}
