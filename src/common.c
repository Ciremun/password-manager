#include "common.h"
#include "sync.h"

extern struct AES_ctx ctx;
extern uint8_t aes_iv[];
extern char *data_store;
extern char *sync_remote_url;

const char *help_s =
    "\n"
    "./pm [flags]                  read or write data\n"
    "\n"
    "sync:                         set PM_SYNC_REMOTE_URL env var\n"
    "\n"
    "flags:\n"
    "\n"
    "-d  --data                    data to encrypt\n"
    "-df --data-file               read data from file\n"
    "-l  --label                   add label for data\n"
    "-fl --find-label              find data by label\n"
    "-dl --delete-label            delete label and its data\n"
#ifdef _WIN32
    "-c  --copy                    -fl, -gp helper, copy to clipboard\n"
#else
    "-c  --copy                    -fl, -gp helper, pipe with clip tools\n"
#endif
    "-gp --generate-password [N]   put random data\n"
    "-k  --key                     key\n"
    "-kf --key-file                key file path\n"
    "-i  --input                   stored data path\n"
    "-v  --version                 display version\n"
    "-h  --help                    display help\n\n";

void input_key(uint8_t **aes_key, Flags *f)
{
    if (!*aes_key)
    {
        if (f != NULL)
        {
            if (!f->copy.exists)
            {
                printf("key?\n");
            }
        }
        else
        {
            printf("key?\n");
        }
        getpasswd((char **)aes_key);
    }
}

void decrypt_and_print(uint8_t *aes_key, Flags *f)
{
    pull_changes(sync_remote_url);
    size_t idx = 0;
    char **lines = NULL;
    read_file(data_store, &lines, &idx);
    input_key(&aes_key, f);
    int did_print = 0;
    for (size_t i = 0; i < idx; i++)
    {
        size_t decsize = 0;
        size_t line_length = strlen(lines[i]);
        unsigned char *decoded_data =
            b64_decode_ex(lines[i], line_length, &decsize);
        AES_init_ctx_iv(&ctx, aes_key, aes_iv);
        AES_CTR_xcrypt_buffer(&ctx, decoded_data, decsize);
        if (f->find_label.value != NULL)
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
            size_t query_len = strlen(f->find_label.value);
            if (query_len > label_length)
            {
                free(label);
                free(decoded_data);
                continue;
            }
            int do_continue = 0;
            for (size_t j = 0; j < query_len; j++)
            {
                if (f->find_label.value[j] != label[j])
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
            if (f->copy.exists)
            {
                const char *password =
                    (const char *)decoded_data + label_length + 1;
#ifdef _WIN32
                if (copy_to_clipboard(password, strlen(password) + 1))
                {
#else
                {
                    printf("%s", password);
#endif
                    did_print = 1;
                    free(decoded_data);
                    break;
                }
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

void encrypt_and_replace(Flags *f, char *find_label, char *data,
                         uint8_t *aes_key)
{
    char **lines = NULL;
    size_t idx = 0;

    FILE *fh = NULL;
    if (!(fh = fopen(data_store, "rb")))
    {
        fh = fopen(data_store, "a");
    }
    fclose(fh);

    read_file(data_store, &lines, &idx);
    input_key(&aes_key, f);

    size_t label_and_data_size = strlen(find_label) + strlen(data) + 2;

    for (size_t i = 0; i < idx; i++)
    {
        size_t decsize = 0;
        size_t line_length = strlen(lines[i]);
        unsigned char *decoded_data =
            b64_decode_ex(lines[i], line_length, &decsize);
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
            snprintf((char *)decoded_data, sizeof(char) * label_and_data_size,
                     "%s %s", label, data);

            AES_init_ctx_iv(&ctx, aes_key, aes_iv);
            AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decoded_data,
                                  label_and_data_size);

            char *encoded_data = b64_encode(decoded_data, label_and_data_size);

            FILE *f = fopen(data_store, "wb");
            if (f == NULL)
            {
                error(stderr, "error opening file %s\n", data_store);
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

            upload_changes(sync_remote_url);
            return;
        }

        free(label);
        free(decoded_data);
    }

    uint8_t *label_and_data = malloc(label_and_data_size);
    snprintf((char *)label_and_data, sizeof(uint8_t) * label_and_data_size,
             "%s %s", find_label, data);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, label_and_data, label_and_data_size);

    char *encoded_data = b64_encode(label_and_data, label_and_data_size);
    write_file(data_store, "a", encoded_data);

    for (size_t i = 0; i < idx; i++)
    {
        free(lines[i]);
    }

    free(lines);
    free(label_and_data);
    free(encoded_data);
    free(aes_key);

    upload_changes(sync_remote_url);
}

void encrypt_and_write(Flags *f, uint8_t *data, uint8_t *aes_key,
                       size_t data_length)
{
    input_key(&aes_key, f);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, data_length);

    char *encoded_data = b64_encode(data, data_length);
    write_file(data_store, "a", encoded_data);
    free(encoded_data);
    free(aes_key);

    upload_changes(sync_remote_url);
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

    if (!(f = fopen(fp, "rb")))
    {
        error(stderr, "error opening file %s\n", fp);
        exit(1);
    }

    if (!(*lines = calloc(LMAX, sizeof(**lines))))
    {
        error(stderr, "%s\n", "error: memory allocation failed");
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
                error(stderr, "%s\n", "error: memory allocation failed");
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
    FILE *f = fopen(fp, "rb");
    if (f == NULL)
    {
        error(stderr, "error opening file %s\n", fp);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char *str = (char *)malloc(size + 1);
    fseek(f, 0, SEEK_SET);
    fread(str, 1, size, f);
    str[size] = '\0';
    fclose(f);
    if (nch != NULL)
        *nch = size;
    return str;
}

void write_file(const char *fp, const char *mode, void *data)
{
    FILE *f = fopen(fp, mode);
    if (f == NULL)
    {
        error(stderr, "error opening file %s\n", fp);
        exit(1);
    }
    fprintf(f, "%s\n", (char *)data);
    fclose(f);
}

void delete_label(char *find_label, uint8_t *aes_key)
{
    int found_label = 0;
    size_t total_lines = 0;
    char **lines = NULL;
    read_file(data_store, &lines, &total_lines);
    input_key(&aes_key, NULL);
    size_t line_idx = 0;
    for (; line_idx < total_lines; ++line_idx)
    {
        size_t decoded_line_length = 0;
        unsigned char *decoded_line =
            decode_line(lines[line_idx], aes_key, &decoded_line_length);
        char *label = malloc(decoded_line_length * sizeof(decoded_line) + 1);
        if (!label)
        {
            PANIC_MALLOC();
        }
        for (size_t j = 0; j < decoded_line_length; j++)
        {
            if (decoded_line[j] == ' ')
            {
                label[j] = '\0';
                break;
            }
            label[j] = decoded_line[j];
        }
        free(decoded_line);
        if (strcmp(label, find_label) == 0)
        {
            found_label = 1;
            free(label);
            break;
        }
        free(label);
    }
    if (found_label)
    {
        FILE *f = 0;
        if (!(f = fopen(data_store, "wb")))
        {
            PANIC_OPEN_FILE(data_store);
        }
        for (size_t i = 0; i < total_lines; ++i)
        {
            if (i == line_idx)
            {
                continue;
            }
            size_t line_length = strlen(lines[i]);
            for (size_t j = 0; j < line_length; ++j)
            {
                putc(lines[i][j], f);
            }
            putc('\n', f);
        }
        fclose(f);
    }
    else
    {
        printf("info: no results\n");
    }
    for (size_t i = 0; i < total_lines; i++)
    {
        free(lines[i]);
    }
    free(lines);
    free(aes_key);

    upload_changes(sync_remote_url);
}

unsigned char *decode_line(const char *line, uint8_t *aes_key,
                           size_t *decoded_line_length)
{
    unsigned char *decoded_line =
        b64_decode_ex(line, strlen(line), decoded_line_length);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decoded_line, *decoded_line_length);
    return decoded_line;
}