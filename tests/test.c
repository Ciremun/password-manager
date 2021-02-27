#include "../src/io/common.h"

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

void test_data_flag()
{
}

int main()
{
    uint8_t *aes_key = calloc(1, 32);
    memcpy(aes_key, "khello", 7);
    char **argv = calloc(1, 256);
    argv[0] = strdup("test.c");
    argv[1] = strdup("-d");
    argv[2] = strdup("data2");
    int argc = 3;
    return run(aes_key, argc, argv);
}
