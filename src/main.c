#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "aes.h"
#include "b64/b64.h"

#define PASSWORDS_STORE ".data"
#define LMAX 255


ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL)
    {
        return -1;
    }
    if (stream == NULL)
    {
        return -1;
    }
    if (n == NULL)
    {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF)
    {
        return -1;
    }
    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
        {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF)
    {
        if ((size_t)(p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
            {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n')
        {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

char** read_file(const char *fp, size_t *lsize)
{
    char **lines = NULL;
    char *ln = NULL;
    size_t n = 0;
    ssize_t nchr = 0;
    size_t idx = 0;
    size_t lmax = LMAX;
    FILE *f = NULL;

    if (!(f = fopen(fp, "r")))
    {
        fprintf(stderr, "error: file open failed '%s'.", fp);
        exit(1);
    }

    if (!(lines = calloc(LMAX, sizeof *lines)))
    {
        fprintf(stderr, "error: memory allocation failed.");
        exit(1);
    }

    while ((nchr = getline(&ln, &n, f)) != -1)
    {
        while (nchr > 0 && (ln[nchr - 1] == '\n' || ln[nchr - 1] == '\r'))
            ln[--nchr] = 0;

        lines[idx++] = strdup(ln);

        if (idx == lmax)
        {
            char **tmp = realloc(lines, lmax * 2 * sizeof *lines);
            if (!tmp)
                exit(1);
            lines = tmp;
            lmax *= 2;
        }
    }

    if (f)
        fclose(f);
    if (ln)
        free(ln);

    *lsize = idx;
    return lines;
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
        size_t idx = 0;
        char **lines = read_file(PASSWORDS_STORE, &idx);
        for (size_t i = 0; i < idx; i++)
        {
            size_t decsize = 0;
            unsigned char *decoded_data = b64_decode_ex(lines[i], strlen(lines[i]), &decsize);

            AES_init_ctx_iv(&ctx, aes_key, aes_iv);
            AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);
            printf("DEC: %s\n", decoded_data);
            free(decoded_data);
        }
        for (size_t it = 0; it < idx; it++)
            free(lines[it]);
        free(lines);
        return 0;
    }

    uint8_t *password = (uint8_t *)argv[1];
    size_t password_length = strlen(argv[1]);

    printf("ENC: %s\n", password);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, password, password_length);

    char *encoded_data = b64_encode(password, password_length);
    write_file(PASSWORDS_STORE, "a", encoded_data);
}
