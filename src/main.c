#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "aes.h"
#include "b64/b64.h"

struct AES_ctx ctx;

void write(const char *filename, const char *mode, void *data)
{
    FILE *f = fopen(filename, mode);
    if (f == NULL)
    {
        printf("Error opening file %s.\n", filename);
        exit(1);
    }
    fprintf(f, "%s\n", (char *)data);
    fclose(f);
}

int main()
{
    uint8_t aes_key[] = "7bf46a2a35655558232417df6887f9cfe57f32a4";
    uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
    uint8_t password[] = "kappa123";

    printf("ENC: %s\n", password);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, password, strlen((char *)password));

    char *encoded_data = b64_encode(password, strlen((char *)password));
    unsigned char *decoded_data = b64_decode(encoded_data, strlen(encoded_data));

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    // TODO(#1): fix password length calc
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, strlen((char *)decoded_data));
    printf("DEC: %s\n", decoded_data);
}
