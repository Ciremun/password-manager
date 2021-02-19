#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "aes.h"
#include "b64/b64.h"

int main()
{
    uint8_t key[] = "hgu575iue3jd8hjncnsgwfr2940";
    // uint8_t in[] = "owoimtryingalongpassWORDthistimeUwU";
    uint8_t iv[16] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

    struct AES_ctx ctx;
    // AES_init_ctx_iv(&ctx, key, iv);
    // printf("ENC: %s\n", in);
    // AES_CTR_xcrypt_buffer(&ctx, in, strlen((char *)in));
    // printf("enc: %s\n", in);

    char *be = "S9SNPDmB3lj58KRC6mckDSlKrpbGhixy7kuLZ69ZfuHQPfg=";
    // char *be = b64_encode(mystr, strlen(mystr));
    printf("be: %s\n", be);
    char *bd = b64_decode(be, strlen(be));
    printf("bd: %s\n", bd);
    // FILE *f = fopen("tttttttt.txt", "w");
    // if (f == NULL)
    // {
    //     printf("Error opening file!\n");
    //     exit(1);
    // }

    // fprintf(f, "%s", in);
    // fclose(f);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer(&ctx, bd, strlen(bd));
    printf("DEC: %s\n", bd);
}