#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

#include "aes.h"
#include "b64/b64.h"

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define MAX_INPUT_LEN 1024
#define PASSWORDS_STORE ".data"
#define LMAX 255

// TODO(#6): label search

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF)
    {
        return -1;
    }

    if (*lineptr == NULL)
    {
        *lineptr = malloc(128);
        if (*lineptr == NULL)
        {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while (c != EOF)
    {
        if (pos + 1 >= *n)
        {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128)
            {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL)
            {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos++] = c;
        if (c == '\n')
        {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

char **read_file(const char *fp, size_t *lsize)
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
        fprintf(stderr, "error: file open failed '%s'.\n", fp);
        exit(1);
    }

    if (!(lines = calloc(LMAX, sizeof *lines)))
    {
        fprintf(stderr, "error: memory allocation failed.\n");
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

void encrypt_and_write(uint8_t *data, uint8_t *aes_key, size_t *data_length)
{
    printf("ENC: %s\n", data);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, *data_length);

    char *encoded_data = b64_encode(data, *data_length);
    write_file(PASSWORDS_STORE, "a", encoded_data);
}

void input_key(uint8_t *aes_key)
{
    printf("key?\n");
    scanf("%" STRINGIFY(MAX_INPUT_LEN) "[^\n]", aes_key);
}

int main(int argc, char **argv)
{
    uint8_t *aes_key = malloc(MAX_INPUT_LEN);

    if (argc == 1)
    {
        size_t idx = 0;
        char **lines = read_file(PASSWORDS_STORE, &idx);
        input_key(aes_key);
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

    char *label = NULL;

    if (argc > 2)
    {
        for (int i = 1; i < argc; i++)
        {
            if ((strcmp("-l", argv[i]) == 0) || (strcmp("--label", argv[i]) == 0))
            {
                label = argv[i + 1];
                i++;
                continue;
            }
        }
    }

    uint8_t *data = NULL;
    size_t data_length = 0;

    if (label != NULL)
    {
        char *last_arg = argv[argc - 1];
        if (strcmp(last_arg, label) == 0)
        {   
            printf("error: label provided but no data\n");
            exit(1);
        }
        data = malloc(MAX_INPUT_LEN * 2 + 2);
        snprintf((char *)data, sizeof(uint8_t) * MAX_INPUT_LEN * 2 + 2, "%s %s", label, last_arg);
        data_length = strlen((char *)data);
        input_key(aes_key);
        encrypt_and_write(data, aes_key, &data_length);
        free(data);
    }
    else
    {
        data = (uint8_t *)argv[argc - 1];
        data_length = strlen((char *)data);
        input_key(aes_key);
        encrypt_and_write(data, aes_key, &data_length);
    }

    free(aes_key);
    return 0;
}
