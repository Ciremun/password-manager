#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "aes.h"

int main()
{
    struct AES_ctx ctx;
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t encr[16];

    memmove(encr, "1234567891111111", 16);

    AES_init_ctx(&ctx, key);
    printf("encr:: %s\n", encr);
    AES_CBC_encrypt_buffer(&ctx, encr, 16);
    AES_CBC_decrypt_buffer(&ctx, encr, 16);
    printf("decr:: %s\n", encr);
}