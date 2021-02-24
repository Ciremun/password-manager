#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include "io/win.h"
#else
#include "io/unix.h"
#endif

#include "io/common.h"

#include "aes.h"
#include "b64/b64.h"

#define MAX_KEY_LEN 1024
#define DATA_STORE ".data"

// TODO(#8): "generate password" flag
// TODO(#11): read data from file
// TODO(#14): replace data for label

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};
const char *help_s = "\n\
    ./pm [flags]        read or write data\n\
\n\
    flags:\n\
\n\
    -d  --data          data to encrypt\n\
    -l  --label         add label for data\n\
    -fl --find-label    find data by label\n\
    -h  --help          display help\n\
\n\
";

void encrypt_and_write(uint8_t *data, uint8_t *aes_key, size_t *data_length)
{
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, *data_length);

    char *encoded_data = b64_encode(data, *data_length);
    write_file(DATA_STORE, "a", encoded_data);
    free(encoded_data);
}

void input_key(uint8_t *aes_key)
{
    printf("key?\n");
    getpasswd((char **)&aes_key, MAX_KEY_LEN);
}

int parse_arg(const char *s, const char *l, char **out, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(s, argv[i]) == 0) || (strcmp(l, argv[i]) == 0))
        {
            if ((i + 1) >= argc)
            {
                return 1;
            }
            *out = argv[i + 1];
            return 1;
        }
    }
    return 0;
}

void decrypt_and_print(uint8_t *aes_key, char *find_label)
{
    ssize_t idx = 0;
    char **lines = NULL;
    read_file(DATA_STORE, &lines, &idx);
    if (idx == -1)
    {
        printf("Error opening file %s.\n", DATA_STORE);
        exit(1);
    }
    input_key(aes_key);
    int did_print = 0;
    for (ssize_t i = 0; i < idx; i++)
    {
        size_t decsize = 0;
        size_t line_length = strlen(lines[i]);
        unsigned char *decoded_data = b64_decode_ex(lines[i], line_length, &decsize);
        AES_init_ctx_iv(&ctx, aes_key, aes_iv);
        AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);
        if (find_label != NULL)
        {
            char *label = malloc(decsize);
            size_t label_length = 0;
            int found_label = 0;
            for (size_t j = 0; j < decsize; j++)
            {
                if (decoded_data[j] == ' ')
                {
                    found_label = 1;
                    label[j] = '\0';
                    label_length++;
                    break;
                }
                label[j] = decoded_data[j];
                label_length++;
            }
            if (!found_label)
            {
                free(label);
                free(decoded_data);
                continue;
            }
            size_t query_len = strlen(find_label);
            if (query_len > label_length)
            {
                free(label);
                free(decoded_data);
                continue;
            }
            int do_continue = 0;
            for (size_t j = 0; j < query_len; j++)
            {
                if (find_label[j] != label[j])
                {
                    do_continue++;
                    break;
                }
            }
            free(label);
            if (do_continue)
            {
                free(decoded_data);
                continue;
            }
        }
        printf("%s\n", decoded_data);
        did_print = 1;
        free(decoded_data);
    }
    if (!did_print)
    {
        printf("info: no results\n");
    }
    for (ssize_t it = 0; it < idx; it++)
        free(lines[it]);
    free(lines);
    free(aes_key);
    exit(0);
}

int main(int argc, char **argv)
{
    uint8_t *aes_key = NULL;

    uint8_t *data = NULL;
    parse_arg("-d", "--data", (char **)&data, argc, argv);

    if (data == NULL)
    {
        char *find_label = NULL;
        parse_arg("-fl", "--find-label", &find_label, argc, argv);

        if (find_label != NULL)
        {
            aes_key = calloc(1, MAX_KEY_LEN);
            decrypt_and_print(aes_key, find_label);
        }
        else
        {
            char *help = NULL;
            int help_flag = parse_arg("-h", "--help", &help, argc, argv);

            if (help_flag)
            {
                printf("%s\n", help_s);
                return 0;
            }

            aes_key = calloc(1, MAX_KEY_LEN);
            decrypt_and_print(aes_key, NULL);
        }
    }

    char *label = NULL;
    parse_arg("-l", "--label", &label, argc, argv);

    aes_key = calloc(1, MAX_KEY_LEN);
    input_key(aes_key);

    if (label != NULL)
    {
        size_t label_and_data_size = strlen(label) + strlen((char *)data);
        uint8_t *label_and_data = malloc(label_and_data_size + 2);
        snprintf((char *)label_and_data, sizeof(uint8_t) * (label_and_data_size + 2), "%s %s", label, data);
        size_t label_and_data_length = strlen((char *)label_and_data);
        encrypt_and_write(label_and_data, aes_key, &label_and_data_length);
        free(label_and_data);
    }
    else
    {
        size_t data_length = strlen((char *)data);
        encrypt_and_write(data, aes_key, &data_length);
    }

    free(aes_key);
    return 0;
}
