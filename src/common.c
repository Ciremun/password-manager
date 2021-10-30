#include "common.h"
#include "mem.h"
#include "sync.h"

extern struct AES_ctx ctx;
extern uint8_t        aes_iv[];
extern char          *data_store;
extern char          *sync_remote_url;
extern Memory         g_mem;

const char *help_s
    = "\n"
      "./pm [flags]                  read or write data\n"
      "\n"
      "sync:                         set PM_SYNC_REMOTE_URL env var\n"
      "\n"
      "flags:\n"
      "\n"
      "-d  --data                    data to encrypt\n"
      "-df --data-file               data to encrypt from file\n"
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
      "-i  --input                   encrypted file path\n"
      "-o  --output                  decrypted file path\n"
      "-b  --binary                  binary mode\n"
      "-v  --version                 display version\n"
      "-h  --help                    display help\n\n";

#ifdef _WIN32
int copy_to_clipboard(const char *password, size_t size)
{
    if (!OpenClipboard(0))
    {
        return 0;
    }
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    memcpy(GlobalLock(hMem), password, size);
    GlobalUnlock(hMem);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
    GlobalFree(hMem);
    return 1;
}
#endif // _WIN32

int getpasswd(char **pw)
{
    int    c = 0;
    size_t idx = 0;
    size_t buf = 128;

    if (*pw == NULL)
        *pw = (char *)alloc(buf);

#ifndef _WIN32
    struct termios old_kbd_mode;
    struct termios new_kbd_mode;

    if (tcgetattr(0, &old_kbd_mode))
    {
        error("%s tcgetattr failed\n", __func__);
        return 0;
    }
    memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr(0, TCSANOW, &new_kbd_mode))
    {
        error("%s tcsetattr failed\n", __func__);
        return 0;
    }
#endif // _WIN32
    while (
#ifdef _WIN32
        (c = _getch()) != 13
#else
        (c = fgetc(stdin)) != '\n'
#endif // _WIN32
        && c != EOF)
    {
        if (c != 127)
        {
            if (idx >= buf)
            {
                buf *= 2;
                alloc(buf);
            }
            (*pw)[idx++] = c;
        }
        else if (idx > 0)
        {
            (*pw)[--idx] = 0;
        }
    }

#ifndef _WIN32
    if (tcsetattr(0, TCSANOW, &old_kbd_mode))
    {
        error("%s tcsetattr failed\n", __func__);
        return 0;
    }
#endif // _WIN32

    return 1;
}

void input_key(uint8_t **aes_key, Flags *f)
{
    if (!*aes_key)
    {
        if (f == NULL || !f->copy.exists)
            fprintf(stdout, "%s\n", "key?");
        getpasswd((char **)aes_key);
    }
}

Lines decrypt_and_find(uint8_t *aes_key, Flags *f)
{
    pull_changes(sync_remote_url);
    size_t nch = 0;
    char  *str = read_file_as_str(data_store, &nch);
    input_key(&aes_key, f);
    if (f->binary.exists)
    {
        decrypt_raw((uint8_t *)str, aes_key, nch);
        Lines lines = {
            .array = (Line *)alloc(sizeof(Line)),
            .count = 1,
        };
        lines.array[0] = (Line){.data = str, .length = nch};
        return lines;
    }
    size_t i = 0;
    size_t query_len
        = f->find_label.value != NULL ? strlen(f->find_label.value) : 0;
    size_t total_lines = 0;
    for (size_t j = 0; j < nch; ++j)
        if (str[j] == '\n')
            total_lines++;
    Lines lines = {
        .array = (Line *)alloc(sizeof(Line) * total_lines),
        .count = 0,
    };
    while (i < nch)
    {
        size_t start = i;
        do
        {
            i++;
        } while (i < nch && str[i] != '\n');
        size_t line_length = i - start - 1;
        if (str[i] == '\n')
            i++;
        size_t         decsize = 0;
        unsigned char *decoded_data
            = decrypt_base64(str + start, aes_key, line_length, &decsize);
        if (f->find_label.value != NULL)
        {
            char  *label = (char *)alloc(decsize);
            size_t label_length = 0;
            int    found_label = 0;
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
                continue;
            }
            if (query_len > label_length)
            {
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
            if (do_continue)
            {
                continue;
            }
            if (f->copy.exists)
            {
                const char *password
                    = (const char *)decoded_data + label_length + 1;
		size_t password_length = decsize - label_length - 1;
#ifdef _WIN32
                if (!copy_to_clipboard(password, password_length))
                {
                    error("%s\n", "couldn't copy to clipboard");
                }
#else
                fwrite(password, sizeof(char), password_length, stdout);
#endif // _WIN32
                exit(0);
            }
        }
        lines.array[lines.count++]
            = (Line){.data = (char *)decoded_data, .length = decsize};
    }
    if (f->copy.exists)
        lines.count = 0;
    return lines;
}

void encrypt_and_replace(Flags *f, char *find_label, char *data,
                         uint8_t *aes_key, size_t data_length)
{
    char **lines = NULL;
    size_t idx = 0;

    FILE *fh = NULL;
    if (!(fh = fopen(data_store, "rb")))
        fh = fopen(data_store, "a");
    if (fh)
        fclose(fh);

    read_file(data_store, &lines, &idx);
    input_key(&aes_key, f);

    if (!data_length)
        data_length = strlen(data);

    size_t find_label_length = strlen(find_label);
    size_t label_and_data_size = find_label_length + data_length + 2;

    for (size_t i = 0; i < idx; i++)
    {
        size_t         decsize = 0;
        size_t         line_length = strlen(lines[i]);
        unsigned char *decoded_data
            = decrypt_base64(lines[i], aes_key, line_length, &decsize);
        alloc(label_and_data_size - decsize);

        char *label = (char *)alloc(decsize);

        size_t label_length;
        for (label_length = 0; label_length < decsize; label_length++)
        {
            if (decoded_data[label_length] == ' ')
            {
                label[label_length] = '\0';
                break;
            }
            label[label_length] = decoded_data[label_length];
        }

        if (strcmp(label, find_label) == 0)
        {
            memcpy(decoded_data, label, label_length);
            decoded_data[label_length] = ' ';
            memcpy(decoded_data + label_length + 1, data, data_length);

            AES_init_ctx_iv(&ctx, aes_key, aes_iv);
            AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decoded_data,
                                  label_and_data_size);

            size_t encsize;
            char  *encoded_data
                = b64_encode(decoded_data, label_and_data_size, &encsize);

            FILE *f = fopen(data_store, "wb");
            if (f == NULL)
                PANIC_OPEN_FILE(data_store);

            memset(lines[i], 0, line_length);
            {
                void *tmp = realloc(lines[i], encsize + 1);
                if (tmp == NULL)
                    PANIC("%s\n", "memory allocation failed!");
                lines[i] = (char *)tmp;
            }
            strcpy(lines[i], encoded_data);

            for (size_t k = 0; k < idx; k++)
            {
                fprintf(f, "%s\n", (char *)lines[k]);
                free(lines[k]);
            }

            fclose(f);
            free(lines);

            upload_changes(sync_remote_url);
            return;
        }
    }

    uint8_t *label_and_data = (uint8_t *)alloc(label_and_data_size);
    memcpy(label_and_data, find_label, find_label_length);
    label_and_data[find_label_length] = ' ';
    memcpy(label_and_data + find_label_length + 1, data, data_length);

    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, label_and_data, label_and_data_size);

    size_t encsize;
    char  *encoded_data
        = b64_encode(label_and_data, label_and_data_size, &encsize);
    write_file(data_store, "a", encoded_data, encsize);

    for (size_t i = 0; i < idx; i++)
        free(lines[i]);

    free(lines);

    upload_changes(sync_remote_url);
}

void encrypt_and_write(Flags *f, uint8_t *data, uint8_t *aes_key,
                       size_t data_length)
{
    input_key(&aes_key, f);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, data, data_length);

    if (!f->binary.exists)
    {
        size_t encsize;
        char  *encoded_data = b64_encode(data, data_length, &encsize);
        write_file(data_store, "a", encoded_data, encsize);
        upload_changes(sync_remote_url);
    }
    else
        write_file(data_store, "a", data, data_length);
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    size_t pos;
    int    c;

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
        *lineptr = (char *)malloc(128);
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
            char *new_ptr = (char *)realloc(*lineptr, new_size);
            if (new_ptr == NULL)
                return -1;
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
    char   *ln = NULL;
    size_t  n = 0;
    ssize_t nchr = 0;
    size_t  idx = 0;
    size_t  lmax = LMAX;
    FILE   *f = NULL;

    if (!(f = fopen(fp, "rb")))
        PANIC_OPEN_FILE(fp);

    if (!(*lines = (char **)calloc(LMAX, sizeof(**lines))))
    {
        error("%s\n", "memory allocation failed");
        exit(1);
    }

    while ((nchr = getline(&ln, &n, f)) != -1)
    {
        while (nchr > 0 && (ln[nchr - 1] == '\n' || ln[nchr - 1] == '\r'))
            ln[--nchr] = 0;

        (*lines)[idx++] = strdup(ln);

        if (idx == lmax)
        {
            char **tmp = (char **)realloc(lines, lmax * 2 * sizeof *lines);
            if (tmp == NULL)
                PANIC("%s\n", "memory allocation failed!");
            *lines = tmp;
            lmax *= 2;
        }
    }

    fclose(f);
    free(ln);

    *lsize = idx;
}

char *read_file_as_str(const char *fp, size_t *nch)
{
    FILE *f = fopen(fp, "rb");
    if (f == NULL)
        PANIC_OPEN_FILE(fp);
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char  *str = (char *)alloc(size + 1);
    fseek(f, 0, SEEK_SET);
    fread(str, 1, size, f);
    str[size] = '\0';
    fclose(f);
    if (nch != NULL)
        *nch = size;
    return str;
}

void write_file(const char *fp, const char *mode, void *data, size_t size)
{
    FILE *f = fopen(fp, mode);
    if (f == NULL)
        PANIC_OPEN_FILE(fp);
    fwrite(data, sizeof(char), size, f);
    fputc('\n', f);
    fclose(f);
}

void delete_label(char *find_label, uint8_t *aes_key)
{
    int    found_label = 0;
    size_t total_lines = 0;
    char **lines = NULL;
    read_file(data_store, &lines, &total_lines);
    input_key(&aes_key, NULL);
    size_t line_idx = 0;
    for (; line_idx < total_lines; ++line_idx)
    {
        size_t         decoded_line_length = 0;
        size_t         line_length = strlen(lines[line_idx]);
        unsigned char *decoded_line = decrypt_base64(
            lines[line_idx], aes_key, line_length, &decoded_line_length);
        char *label
            = (char *)alloc(decoded_line_length * sizeof(decoded_line) + 1);
        for (size_t j = 0; j < decoded_line_length; j++)
        {
            if (decoded_line[j] == ' ')
            {
                label[j] = '\0';
                break;
            }
            label[j] = decoded_line[j];
        }
        if (strcmp(label, find_label) == 0)
        {
            found_label = 1;
            break;
        }
    }
    if (found_label)
    {
        FILE *f = 0;
        if (!(f = fopen(data_store, "wb")))
            PANIC_OPEN_FILE(data_store);
        for (size_t i = 0; i < total_lines; ++i)
        {
            if (i == line_idx)
                continue;
            size_t line_length = strlen(lines[i]);
            for (size_t j = 0; j < line_length; ++j)
                putc(lines[i][j], f);
            putc('\n', f);
        }
        fclose(f);
    }
    else
    {
        info("%s\n", "no results");
    }
    for (size_t i = 0; i < total_lines; i++)
        free(lines[i]);
    free(lines);

    upload_changes(sync_remote_url);
}

unsigned char *decrypt_base64(const char *line, uint8_t *aes_key,
                              size_t line_length, size_t *decoded_line_length)
{
    unsigned char *decoded_line
        = b64_decode_ex(line, line_length, decoded_line_length);
    decrypt_raw((uint8_t *)decoded_line, aes_key, *decoded_line_length);
    return decoded_line;
}

void decrypt_raw(uint8_t *line, uint8_t *aes_key, size_t length)
{
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, line, length);
}

void decrypt_and_print(uint8_t *aes_key, Flags *f)
{
    Lines lines = decrypt_and_find(aes_key, f);
    if (lines.count)
    {
        FILE *o;
        if (f->output.exists)
        {
            if (f->output.value)
            {
                o = fopen(f->output.value, "wb");
                if (o == NULL)
                {
                    PANIC_OPEN_FILE(f->output.value);
                }
            }
            else
            {
                error("%s\n", "output flag called without filename");
                return;
            }
        }
        else
        {
            o = stdout;
        }
        for (size_t i = 0; i < lines.count; ++i)
        {
            fwrite(lines.array[i].data, sizeof(char), lines.array[i].length, o);
            fputc('\n', o);
        }
        if (f->output.exists)
        {
            fclose(o);
        }
    }
    else
    {
        info("%s\n", "no results");
    }
}

void *alloc(u64 size)
{
    void *tmp = mem_alloc(&g_mem, size);
    if (tmp == NULL)
        PANIC("%s\n", "mem_alloc failed!");
    return tmp;
}
