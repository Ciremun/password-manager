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

    // TODO(#1): fix password length calc
    // encrypted data may contain the '\0'
    // ENC: HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHellWorldHelloWor
    // password_length: 97
    // ```console
    // i: 0, data: 179
    // i: 1, data: 38
    // i: 2, data: 59
    // i: 3, data: 171
    // i: 4, data: 73
    // i: 5, data: 127
    // i: 6, data: 83
    // i: 7, data: 128
    // i: 8, data: 144
    // i: 9, data: 230
    // i: 10, data: 33
    // i: 11, data: 162
    // i: 12, data: 133
    // i: 13, data: 22
    // i: 14, data: 49
    // i: 15, data: 65
    // i: 16, data: 244
    // i: 17, data: 115
    // i: 18, data: 56
    // i: 19, data: 51
    // i: 20, data: 56
    // i: 21, data: 227
    // i: 22, data: 44
    // i: 23, data: 220
    // i: 24, data: 143
    // i: 25, data: 18
    // i: 26, data: 247
    // i: 27, data: 77
    // i: 28, data: 222
    // i: 29, data: 43
    // i: 30, data: 152
    // i: 31, data: 98
    // i: 32, data: 143
    // i: 33, data: 152
    // i: 34, data: 240
    // i: 35, data: 33
    // i: 36, data: 248
    // i: 37, data: 140
    // i: 38, data: 30
    // i: 39, data: 0
    // password_data_length: 39
    // DEC: HelloWorldHelloWorldHelloWorldHelloWorl
    // ```

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, decoded_data, strlen((char *)decoded_data));
    printf("DEC: %s\n", decoded_data);
}
