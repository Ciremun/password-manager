#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include "common.h"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
                    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
char *data_store = 0;

int main(int argc, char **argv)
{
    uint8_t *aes_key = NULL;
    return run(aes_key, argc, argv);
}
