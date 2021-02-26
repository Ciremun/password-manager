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
    -df --data-file     read data from file\n\
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
    Flag data_file;
    Flag help;
} Flags;

void input_key(uint8_t **aes_key)
{
    printf("key?\n");
    *aes_key = calloc(1, MAX_KEY_LEN);
    getpasswd((char **)aes_key, MAX_KEY_LEN);
}

void encrypt_and_replace(char *find_label, char *data, uint8_t *aes_key)
{
    char **lines = NULL;
    size_t idx = 0;

    FILE *f = NULL;
    if (!(f = fopen(DATA_STORE, "r")))
    {
        f = fopen(DATA_STORE, "a");
    }
    fclose(f);

    read_file(DATA_STORE, &lines, &idx);
    input_key(&aes_key);

    size_t label_and_data_size = strlen(find_label) + strlen(data) + 2;

    for (size_t i = 0; i < idx; i++)
    {
        size_t decsize = 0;
        size_t line_length = strlen(lines[i]);
        unsigned char *decoded_data = b64_decode_ex(lines[i], line_length, &decsize);
        AES_init_ctx_iv(&ctx, aes_key, aes_iv);
        AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);

        char *label = malloc(decsize);
        for (size_t j = 0; j < decsize; j++)
        {
            if (decoded_data[j] == ' ')
            {
                label[j] = '\0';
                break;
            }
            label[j] = decoded_data[j];
        }

        if (strcmp(label, find_label) == 0)
        {
            memset(decoded_data, 0, decsize);

            decoded_data = realloc(decoded_data, label_and_data_size);
            snprintf((char *)decoded_data, sizeof(char) * label_and_data_size, "%s %s", label, data);
            
            AES_init_ctx_iv(&ctx, aes_key, aes_iv);
            AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decoded_data, label_and_data_size);

            char *encoded_data = b64_encode(decoded_data, label_and_data_size);

            FILE *f = fopen(DATA_STORE, "w");
            if (f == NULL)
            {
                printf("Error opening file %s.\n", DATA_STORE);
                exit(1);
            }

            memset(lines[i], 0, line_length);
            lines[i] = realloc(lines[i], strlen(encoded_data) + 1);
            strcpy(lines[i], encoded_data);

            for (size_t k = 0; k < idx; k++)
            {
                fprintf(f, "%s\n", (char *)lines[k]);
                free(lines[k]);
            }

            fclose(f);
            free(label);
            free(lines);
            free(encoded_data);
            free(decoded_data);
            free(aes_key);

            exit(0);
        }

        free(label);
        free(decoded_data);
    }

    uint8_t *label_and_data = malloc(label_and_data_size);
    snprintf((char *)label_and_data, sizeof(uint8_t) * label_and_data_size, "%s %s", find_label, data);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, label_and_data, label_and_data_size);

    char *encoded_data = b64_encode(label_and_data, label_and_data_size);
    write_file(DATA_STORE, "a", encoded_data);

    for (size_t i = 0; i < idx; i++)
        free(lines[i]);

    free(lines);
    free(label_and_data);
    free(encoded_data);
    free(aes_key);
    exit(0);

}

void encrypt_and_write(uint8_t *data, uint8_t *aes_key, size_t data_length)
{
    input_key(&aes_key);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, data_length);

    char *encoded_data = b64_encode(data, data_length);
    write_file(DATA_STORE, "a", encoded_data);
    free(encoded_data);
    free(aes_key);
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
        else if (!f->data_file.exists  && is_flag(argv[i], "-df", "--data-file"))
            flag = &f->data_file;

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
    Flags f = {0};

    parse_flags(&f, argc, argv);

    if (!f.data.exists)
    {
        if (f.data_file.exists)
        {
            if (!f.data_file.value)
            {
                printf("error: data-file flag called without filename\n");
                return 0;
            }

            size_t nch = 0;
            char *data = read_file_as_str(f.data_file.value, &nch);

            if (f.label.exists)
            {
                encrypt_and_replace(f.label.value, data, aes_key);
            }
            else
            {
                encrypt_and_write((uint8_t *)data, aes_key, nch);
            }

            free(data);
            return 0;
        }
        if (f.label.exists)
        {
            printf("error: label flag called without --data or --data-file\n");
            return 1;
        }
        if (f.find_label.exists)
        {
            if (!f.find_label.value)
            {
                printf("error: find label flag called without name\n");
                return 1;
            }
            decrypt_and_print(aes_key, f.find_label.value);
        }
        else
        {
            if (f.help.exists)
            {
                printf("%s\n", help_s);
                return 0;
            }

            decrypt_and_print(aes_key, NULL);
        }
    }

    if (!f.data.value)
    {
        printf("error: data flag called without data\n");
        return 1;
    }

    if (f.data_file.exists)
    {
        printf("error: can't combine data and data-file flags\n");
        return 1;
    }

    if (f.label.exists)
    {
        if (!f.label.value)
        {
            printf("error: label flag called without name\n");
            return 1;
        }
        encrypt_and_replace(f.label.value, f.data.value, aes_key);
    }
    else
    {
        encrypt_and_write((uint8_t *)f.data.value, aes_key, strlen(f.data.value) + 1);
    }

   return 0;
}
