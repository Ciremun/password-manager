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

typedef struct
{
    char *value;
    int exists;
} Flag;

typedef struct
{
    Flag data;
    Flag label;
    Flag find_label;
    Flag help;
} Flags;

void encrypt_and_write(uint8_t *data, uint8_t *aes_key, size_t *data_length)
{
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, *data_length);

    char *encoded_data = b64_encode(data, *data_length);
    write_file(DATA_STORE, "a", encoded_data);
    free(encoded_data);
}

void input_key(uint8_t **aes_key)
{
    printf("key?\n");
    *aes_key = calloc(1, MAX_KEY_LEN);
    getpasswd((char **)&*aes_key, MAX_KEY_LEN);
}

int is_flag(char *arg, char *s, char *l)
{
    return (strcmp(s, arg) == 0) || (strcmp(l, arg) == 0);
}

void parse_flags(Flags *f, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        Flag *flag = NULL;
        if      (!f->data.exists       && is_flag(argv[i], "-d", "--data"))
            flag = &f->data;
        else if (!f->label.exists      && is_flag(argv[i], "-l", "--label"))
            flag = &f->label;
        else if (!f->find_label.exists && is_flag(argv[i], "-fl", "--find-label"))
            flag = &f->find_label;
        else if (!f->help.exists       && is_flag(argv[i], "-h", "--help"))
            flag = &f->help;

        if (flag != NULL)
        {
            flag->exists = 1;
            if ((i + 1) >= argc)
            {
                continue;
            }
            flag->value = argv[i + 1];
        }
    }
}

void decrypt_and_print(uint8_t *aes_key, char *find_label)
{
    size_t idx = 0;
    char **lines = NULL;
    read_file(DATA_STORE, &lines, &idx);
    input_key(&aes_key);
    int did_print = 0;
    for (size_t i = 0; i < idx; i++)
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
                label_length++;
                if (decoded_data[j] == ' ')
                {
                    found_label = 1;
                    label[j] = '\0';
                    break;
                }
                label[j] = decoded_data[j];
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
    for (size_t it = 0; it < idx; it++)
        free(lines[it]);
    free(lines);
    free(aes_key);
    exit(0);
}

int main(int argc, char **argv)
{
    uint8_t *aes_key = NULL;
    Flags flags = {0};

    parse_flags(&flags, argc, argv);

    if (!flags.data.exists)
    {
        if (flags.label.exists)
        {
            printf("error: label flag called without --data\n");
            return 1;
        }
        if (flags.find_label.exists)
        {
            decrypt_and_print(aes_key, flags.find_label.value);
        }
        else
        {
            if (flags.help.exists)
            {
                printf("%s\n", help_s);
                return 0;
            }

            decrypt_and_print(aes_key, NULL);
        }
    }

    input_key(&aes_key);

    if (flags.label.exists)
    {
        size_t label_and_data_size = strlen(flags.label.value) + strlen((char *)flags.data.value);
        uint8_t *label_and_data = malloc(label_and_data_size + 2);
        snprintf((char *)label_and_data, sizeof(uint8_t) * (label_and_data_size + 2), "%s %s", flags.label.value, flags.data.value);
        size_t label_and_data_length = strlen((char *)label_and_data);
        encrypt_and_write(label_and_data, aes_key, &label_and_data_length);
        free(label_and_data);
    }
    else
    {
        size_t data_length = strlen((char *)flags.data.value);
        encrypt_and_write((uint8_t *)flags.data.value, aes_key, &data_length);
    }

    free(aes_key);
    return 0;
}
