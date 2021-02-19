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
    fprintf(f, "%s\n", data);
    fclose(f);
}

int main()
{
    uint8_t key[] = "7bf46a2a35655558232417df6887f9cfe57f32a4";
    uint8_t in[] = "kappa123";
    uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

    AES_init_ctx_iv(&ctx, key, iv);
    printf("ENC: %s\n", in);
    AES_CTR_xcrypt_buffer(&ctx, in, strlen((char *)in));
    printf("enc: %s\n", in);

    // char *be = "";
    char *be = b64_encode(in, strlen((char *)in));
    printf("be: %s\n", be);

    write(".data", "a", be);

    char *bd = b64_decode(be, strlen(be));
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer(&ctx, bd, strlen(bd));
    printf("DEC: %s\n", bd);
}