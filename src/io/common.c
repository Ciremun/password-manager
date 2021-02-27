#include "common.h"

extern struct AES_ctx ctx;
extern uint8_t aes_iv[];

const char *help_s = "\n\
    ./pm [flags]        read or write data\n\
\n\
    flags:\n\
\n\
    -d  --data                  data to encrypt\n\
    -df --data-file             read data from file\n\
    -l  --label                 add label for data\n\
    -fl --find-label            find data by label\n\
    -gp --generate-password     put random data\n\
    -h  --help                  display help\n\
\n\
";

void input_key(uint8_t **aes_key)
{
    if (!*aes_key)
    {
        printf("key?\n");
        *aes_key = calloc(1, MAX_KEY_LEN);
        getpasswd((char **)aes_key, MAX_KEY_LEN);
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
    {
        free(lines[it]);
    }
    free(lines);
    free(aes_key);
    exit(0);
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
                printf("error opening file %s\n", DATA_STORE);
                return;
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

            return;
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
    {
        free(lines[i]);
    }

    free(lines);
    free(label_and_data);
    free(encoded_data);
    free(aes_key);
    return;
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

void read_file(const char *fp, char ***lines, size_t *lsize)
{
    char *ln = NULL;
    size_t n = 0;
    ssize_t nchr = 0;
    size_t idx = 0;
    size_t lmax = LMAX;
    FILE *f = NULL;

    if (!(f = fopen(fp, "r")))
    {
        printf("error opening file %s\n", fp);
        exit(1);
    }

    if (!(*lines = calloc(LMAX, sizeof(**lines))))
    {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(1);
    }

    while ((nchr = getline(&ln, &n, f)) != -1)
    {
        while (nchr > 0 && (ln[nchr - 1] == '\n' || ln[nchr - 1] == '\r'))
            ln[--nchr] = 0;

        (*lines)[idx++] = strdup(ln);

        if (idx == lmax)
        {
            char **tmp = realloc(lines, lmax * 2 * sizeof *lines);
            if (!tmp)
            {
                fprintf(stderr, "error: memory allocation failed\n");
                exit(1);
            }
            *lines = tmp;
            lmax *= 2;
        }
    }

    if (f)
        fclose(f);
    if (ln)
        free(ln);

    *lsize = idx;
}

char *read_file_as_str(const char *fp, size_t *nch)
{
    FILE *f = fopen(fp, "r");
    if (f == NULL)
    {
        printf("error opening file %s\n", fp);
        exit(1);
    }
    int c;
    size_t size = 1024;
    char *buf = malloc(size);
    if (buf == NULL)
    {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(1);
    }

    while ((c = getc(f)) != EOF)
    {
        if (*nch >= size - 1)
        {
            size *= 2;
            buf = realloc(buf, size);
            if (buf == NULL)
            {
                fprintf(stderr, "error: memory allocation failed\n");
                exit(1);
            }
        }
        buf[(*nch)++] = c;
    }

    buf[(*nch)++] = 0;
    fclose(f);
    return buf;
}

void write_file(const char *fp, const char *mode, void *data)
{
    FILE *f = fopen(fp, mode);
    if (f == NULL)
    {
        printf("Error opening file %s\n", fp);
        exit(1);
    }
    fprintf(f, "%s\n", (char *)data);
    fclose(f);
}
