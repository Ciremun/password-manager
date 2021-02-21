#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>

#include "aes.h"
#include "b64/b64.h"

#define MAX_INPUT_LEN 1024
#define PASSWORDS_STORE ".data"
#define LMAX 255

// TODO(#8): "generate password" flag

struct AES_ctx ctx;
uint8_t aes_iv[] = {0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

ssize_t getpasswd(char **pw, size_t sz, FILE *fp)
{
    if (!pw || !sz || !fp)
        return -1;

    if (*pw == NULL)
    {
        void *tmp = realloc(*pw, sz * sizeof **pw);
        if (!tmp)
            return -1;
        memset(tmp, 0, sz);
        *pw = (char *)tmp;
    }

    size_t idx = 0;
    int c = 0;

    struct termios old_kbd_mode;
    struct termios new_kbd_mode;

    if (tcgetattr(0, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcgetattr failed.\n", __func__);
        return -1;
    }
    memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr(0, TCSANOW, &new_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    while (((c = fgetc(fp)) != '\n' && c != EOF && idx < sz - 1) ||
           (idx == sz - 1 && c == 127))
    {
        if (c != 127)
        {
            (*pw)[idx++] = c;
        }
        else if (idx > 0)
        {
            (*pw)[--idx] = 0;
        }
    }
    (*pw)[idx] = 0;

    if (tcsetattr(0, TCSANOW, &old_kbd_mode))
    {
        fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
        return -1;
    }

    if (idx == sz - 1 && c != '\n')
        fprintf(stderr, " (%s() warning: truncated at %zu chars.)\n",
                __func__, sz - 1);

    return idx;
}

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
    printf("%s\n", data);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, *data_length);

    char *encoded_data = b64_encode(data, *data_length);
    write_file(PASSWORDS_STORE, "a", encoded_data);
}

void input_key(uint8_t *aes_key)
{
    printf("key?\n");
    getpasswd((char **)&aes_key, MAX_INPUT_LEN, stdin);
}

void parse_arg(const char *s, const char *l, char **label, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(s, argv[i]) == 0) || (strcmp(l, argv[i]) == 0))
        {
            if ((i + 1) >= argc)
            {
                *label = argv[argc - 1];
                return;
            }
            *label = argv[i + 1];
            i++;
            continue;
        }
    }
}

void decrypt_and_print(uint8_t *aes_key, char *find_label)
{
    size_t idx = 0;
    char **lines = read_file(PASSWORDS_STORE, &idx);
    input_key(aes_key);
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
            char *label = malloc(MAX_INPUT_LEN);
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
                continue;
            }
            size_t query_len = strlen(find_label);
            if (query_len > label_length)
            {
                free(label);
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
    exit(0);
}

int main(int argc, char **argv)
{
    uint8_t *aes_key = malloc(MAX_INPUT_LEN);

    if (argc == 1)
    {
        decrypt_and_print(aes_key, NULL);
    }

    char *find_label = NULL;
    parse_arg("-fl", "--find-label", &find_label, argc, argv);

    if (find_label != NULL)
    {
        decrypt_and_print(aes_key, find_label);
    }

    char *label = NULL;
    parse_arg("-l", "--label", &label, argc, argv);

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
