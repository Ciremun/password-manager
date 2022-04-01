// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "console/xcrypt.h"
#include "console/aes.h"
#include "console/b64.h"
#include "console/io.h"
#include "console/sync.h"
#include "console/thread.h"

extern struct AES_ctx ctx;
extern uint8_t aes_iv[];
extern String sync_remote_url;
extern char *data_store;

void encrypt_and_replace(Flags *fl, String s, String label, uint8_t *aes_key)
{
    pull_changes(sync_remote_url);

    File f = create_file(data_store, PM_READ_WRITE);
    input_key(aes_key, fl);

    if (f.size == 0)
        goto append;
    else
        MAP_FILE_(&f);

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (f.start[line_end] == '\n')
        {
            size_t line_len = line_end - line_start;
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + line_start, line_len, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            if ((label.length + 1 >= b64_decoded_len) ||
                (memcmp(b64_decoded_str, label.data, label.length) != 0))
                goto skip_line;
            size_t label_len = 0;
            while (b64_decoded_str[++label_len] != ' ')
                if (label_len > b64_decoded_len)
                    goto skip_line;
            if (label_len != label.length)
                goto skip_line;
            size_t label_and_data_length = label_len + 1 + s.length;
            uint8_t *label_and_data = (uint8_t *)calloc(1, label_and_data_length);
            ASSERT_ALLOC(label_and_data);
            memcpy(label_and_data, b64_decoded_str, label_len + 1);
            memcpy(label_and_data + label_len + 1, s.data, s.length);
            xcrypt_buffer(label_and_data, aes_key, label_and_data_length);
            size_t b64_encoded_len;
            char *b64_encoded_str = b64_encode(label_and_data, label_and_data_length, &b64_encoded_len);
            if (b64_encoded_len == line_len)
            {
                memcpy(f.start + line_start, b64_encoded_str, b64_encoded_len);
                UNMAP_AND_CLOSE_FILE(f);
            }
            else
            {
                size_t initial_size = f.size;
                if (b64_encoded_len > line_len)
                {
                    size_t b64_len_diff = b64_encoded_len - line_len;
                    UNMAP_FILE(f);
                    TRUNCATE_FILE(&f, f.size + b64_len_diff);
                    MAP_FILE_(&f);
                    memcpy(f.start + line_end + 1 + b64_len_diff, f.start + line_end + 1, initial_size - line_end - 1);
                    memcpy(f.start + line_start, b64_encoded_str, b64_encoded_len);
                    f.start[line_start + b64_encoded_len] = '\n';
                    UNMAP_AND_CLOSE_FILE(f);
                }
                if (b64_encoded_len < line_len)
                {
                    size_t b64_len_diff = line_len - b64_encoded_len;
                    memcpy(f.start + line_start, b64_encoded_str, b64_encoded_len);
                    f.start[line_start + b64_encoded_len] = '\n';
                    memcpy(f.start + line_start + b64_encoded_len + 1, f.start + line_end + 1, initial_size - line_end - 1);
                    UNMAP_FILE(f);
                    TRUNCATE_FILE(&f, f.size - b64_len_diff);
                    CLOSE_FILE(f.handle);
                }
            }
            free(b64_decoded_str);
            free(label_and_data);
            free(b64_encoded_str);
            goto end;
        skip_line:
            line_start = line_end + 1;
            free(b64_decoded_str);
        }
        line_end++;
    } while (line_end < f.size);

append:
{
    size_t label_and_data_length = label.length + 1 + s.length;
    uint8_t *label_and_data = (uint8_t *)calloc(1, label_and_data_length);
    ASSERT_ALLOC(label_and_data);

    memcpy(label_and_data, label.data, label.length);
    label_and_data[label.length] = ' ';
    memcpy(label_and_data + label.length + 1, s.data, s.length);
    xcrypt_buffer(label_and_data, aes_key, label_and_data_length);

    size_t b64_encoded_len;
    char *b64_encoded_str = b64_encode(label_and_data, label_and_data_length, &b64_encoded_len);
    size_t initial_size = f.size;

    TRUNCATE_FILE(&f, f.size + b64_encoded_len + 1);
    MAP_FILE_(&f);
    memcpy(f.start + initial_size, b64_encoded_str, b64_encoded_len);
    f.start[initial_size + b64_encoded_len] = '\n';

    free(label_and_data);
    free(b64_encoded_str);
}
    UNMAP_AND_CLOSE_FILE(f);
end:
    upload_changes(sync_remote_url);
}

#if PM_THREAD_COUNT > 1
static void* xcrypt_and_write_load(void* parameter)
{
    xcrypt_load_info *xl = (xcrypt_load_info *)parameter;
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, xl->aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, xl->str.data + xl->offset, xl->tl.load);
    memcpy(xl->file.start + xl->offset, xl->str.data + xl->offset, xl->tl.load);
    free(xl);
    return 0;
}
#endif // PM_THREAD_COUNT

void encrypt_and_write(Flags *fl, String s, uint8_t *aes_key)
{
    if (!fl->binary.exists)
        pull_changes(sync_remote_url);

    File f = create_file(data_store, PM_READ_WRITE);
    input_key(aes_key, fl);

    if (fl->binary.exists)
    {
        TRUNCATE_FILE(&f, s.length);
        MAP_FILE_(&f);
#if PM_THREAD_COUNT == 1
        xcrypt_buffer(s.data, aes_key, s.length);
        memcpy(f.start, s.data, s.length);
#else
        xcrypt_load_info xl = { .tl = calc_thread_load(PM_THREAD_COUNT, s.length), .str = s, .file = f, .aes_key = aes_key, .offset = 0 };
        og_thread_t threads[PM_THREAD_COUNT];
        int i = 0;
        if (xl.tl.remainder)
            xl.tl.thread_count -= 1;
        for (; i < xl.tl.thread_count; ++i)
        {
            xcrypt_load_info *xl_copy = (xcrypt_load_info *)malloc(sizeof(xcrypt_load_info));
            memcpy(xl_copy, &xl, sizeof(xcrypt_load_info));
            threads[i] = OGCreateThread(xcrypt_and_write_load, xl_copy);
            xl.offset += xl.tl.load;
        }
        if (xl.tl.remainder)
        {
            xl.tl.load += xl.tl.remainder;
            xcrypt_load_info *xl_copy = (xcrypt_load_info *)malloc(sizeof(xcrypt_load_info));
            memcpy(xl_copy, &xl, sizeof(xcrypt_load_info));
            threads[i] = OGCreateThread(xcrypt_and_write_load, xl_copy);
            xl.tl.thread_count += 1;
        }
        for (int i = 0; i < xl.tl.thread_count; ++i)
            OGJoinThread(threads[i]);
#endif // PM_THREAD_COUNT
        UNMAP_AND_CLOSE_FILE(f);
    }
    else
    {
        xcrypt_buffer(s.data, aes_key, s.length);
        size_t b64_encoded_len;
        char *b64_encoded_str = b64_encode(s.data, s.length, &b64_encoded_len);
        size_t initial_size = f.size;
        TRUNCATE_FILE(&f, f.size + b64_encoded_len + 1);
        MAP_FILE_(&f);
        memcpy(f.start + initial_size, b64_encoded_str, b64_encoded_len);
        free(b64_encoded_str);
        f.start[initial_size + b64_encoded_len] = '\n';
        UNMAP_AND_CLOSE_FILE(f);
        upload_changes(sync_remote_url);
    }
}

void delete_label(Flags *fl, String label, uint8_t *aes_key)
{
    pull_changes(sync_remote_url);

    File f = open_file(data_store, PM_READ_WRITE);

    if (f.size == 0)
    {
        error("file %s is empty", data_store);
        CLOSE_FILE(f.handle);
        return;
    }
    else
        MAP_FILE_(&f);

    input_key(aes_key, fl);

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (f.start[line_end] == '\n')
        {
            size_t line_len = line_end - line_start;
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + line_start, line_len, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            if ((label.length + 1 >= b64_decoded_len) ||
                (memcmp(b64_decoded_str, label.data, label.length) != 0))
                goto skip_line;
            size_t label_len = 0;
            while (b64_decoded_str[++label_len] != ' ')
                if (label_len > b64_decoded_len)
                    goto skip_line;
            free(b64_decoded_str);
            memcpy(f.start + line_start, f.start + line_end + 1, f.size - line_end - 1);
            UNMAP_FILE(f);
            TRUNCATE_FILE(&f, f.size - line_len - 1);
            CLOSE_FILE(f.handle);
            goto end;
        skip_line:
            line_start = line_end + 1;
            free(b64_decoded_str);
        }
        line_end++;
    } while (line_end < f.size);

    info("%s", "no results");
    UNMAP_AND_CLOSE_FILE(f);
end:
    upload_changes(sync_remote_url);
}

void xcrypt_buffer(uint8_t *line, uint8_t *aes_key, size_t length)
{
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    AES_CTR_xcrypt_buffer(&ctx, line, length);
}

void decrypt_and_print(Flags *fl, uint8_t *aes_key)
{
    pull_changes(sync_remote_url);
    FILE *o;
    if (fl->output.exists)
    {
        if (fl->output.value)
        {
            o = fopen(fl->output.value, "wb");
            if (o == NULL)
            {
                error("opening file (%s: %s)", fl->output.value, strerror(errno));
                exit(1);
            }
        }
        else
        {
            error("%s", "output flag called without filename");
            return;
        }
    }
    else
    {
        o = stdout;
    }

    File f = open_file(data_store, PM_READ_ONLY);

    if (f.size == 0)
    {
        error("file %s is empty", data_store);
        CLOSE_FILE(f.handle);
        if (fl->output.exists)
            fclose(o);
        return;
    }
    else
        MAP_FILE_(&f);

    input_key(aes_key, fl);

    int found_label = 0;
    if (fl->binary.exists)
    {
        uint8_t *file_copy = calloc(1, f.size);
        ASSERT_ALLOC(file_copy);
        memcpy(file_copy, f.start, f.size);
        xcrypt_buffer(file_copy, aes_key, f.size);
        setvbuf(o, NULL, _IONBF, 0);
        if (fwrite(file_copy, 1, f.size, o) != f.size)
            error("%s", "fwrite failed");
        free(file_copy);
        goto end;
    }

    size_t find_label_len = 0;
    if (fl->label.exists)
        find_label_len = strlen(fl->label.value);

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (f.start[line_end] == '\n')
        {
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + line_start, line_end - line_start, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            if (fl->label.exists)
            {
                if ((find_label_len + 1 >= b64_decoded_len) ||
                    (memcmp(b64_decoded_str, fl->label.value, find_label_len) != 0))
                    goto skip_write;
                size_t label_len = 0;
                while (b64_decoded_str[++label_len] != ' ')
                    if (label_len > b64_decoded_len)
                        goto skip_write;
                found_label = 1;
                if (fl->copy.exists)
                {
                    if (!copy_to_clipboard(b64_decoded_str + label_len + 1, b64_decoded_len - label_len))
                        error("%s", "couldn't copy to clipboard");
                    free(b64_decoded_str);
                    goto end;
                }
            }
            if (fwrite(b64_decoded_str, 1, b64_decoded_len, o) != b64_decoded_len)
                error("%s", "fwrite failed");
            fputc('\n', o);
        skip_write:
            line_start = line_end + 1;
            free(b64_decoded_str);
        }
        line_end++;
    } while (line_end < f.size);
    fflush(o);
end:
    if (fl->label.exists && !found_label)
        info("%s", "no results");
    if (fl->output.exists)
        fclose(o);
    UNMAP_AND_CLOSE_FILE(f);
}
