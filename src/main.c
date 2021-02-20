#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "aes.h"
#include "b64/b64.h"

#define PASSWORD_LEN 1024
// TODO(#2): read file using realloc()
#define MAX_PASSWORDS 256
#define PASSWORDS_STORE ".data"

int read_file(const char *fp, char lines[][PASSWORD_LEN])
{
    FILE *f = fopen(fp, "r");
    if (f == NULL)
    {
        printf("Error opening file %s.\n", fp);
        exit(1);
    }
    int line = 0;
    while (fgets(lines[line], PASSWORD_LEN, f))
    {
        line++;
    }
    fclose(f);
    return line;
}

void write_file(const char *fp, const char *mode, void *data)
{
    FILE *f = fopen(fp, mode);
    if (f == NULL)
    {
        printf("Error opening file %s.\n", fp);
        exit(1);
    }
    fprintf(f, "%s\n", (char *)data);
    fclose(f);
}

int main(int argc, char **argv)
{
    struct AES_ctx ctx;
    uint8_t aes_key[] = "7bf46a2a35655558232417df6887f9cfe57f32a4";
    uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

    if (argc < 2)
    {
        char lines[MAX_PASSWORDS][PASSWORD_LEN];
        int line_count = read_file(PASSWORDS_STORE, lines);
        for (int i = 0; i < line_count; i++)
        {
            size_t decsize = 0;
            unsigned char *decoded_data = b64_decode_ex(lines[i], strlen(lines[i]), &decsize);

            AES_init_ctx_iv(&ctx, aes_key, aes_iv);
            AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);
            printf("DEC: %s\n", decoded_data);
            free(decoded_data);
        }
        return 0;
    }

    uint8_t *password = (uint8_t *)argv[1];
    size_t password_length = strlen(argv[1]);

    if (password_length > PASSWORD_LEN)
    {
        printf("error: password length > %d\n", PASSWORD_LEN);
        exit(1);
    }

    printf("ENC: %s\n", password);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, password, password_length);

    char *encoded_data = b64_encode(password, password_length);
    write_file(PASSWORDS_STORE, "a", encoded_data);
}
