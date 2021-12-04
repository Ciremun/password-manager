#include "pm_xcrypt.h"
#include "pm_b64.h"
#include "pm_io.h"
#include "pm_sync.h"

extern struct AES_ctx ctx;
extern uint8_t aes_iv[];
extern String sync_remote_url;

void encrypt_and_replace(Flags *fl, String s, String label, uint8_t *aes_key)
{
    // char **lines = NULL;
    // size_t idx = 0;

    // FILE *fh = NULL;
    // if (!(fh = fopen(data_store, "rb")))
    //     fh = fopen(data_store, "a");
    // if (fh)
    //     fclose(fh);

    // read_file(data_store, &lines, &idx);
    // input_key(&aes_key, f);

    // if (!data_length)
    //     data_length = strlen(data);

    // size_t find_label_length = strlen(find_label);
    // size_t label_and_data_size = find_label_length + data_length + 2;

    // for (size_t i = 0; i < idx; i++)
    // {
    //     size_t decsize = 0;
    //     size_t line_length = strlen(lines[i]);
    //     unsigned char *decoded_data = decrypt_base64(lines[i], aes_key, line_length, &decsize);

    //     char *label = (char *)malloc(decsize);

    //     size_t label_length;
    //     for (label_length = 0; label_length < decsize; label_length++)
    //     {
    //         if (decoded_data[label_length] == ' ')
    //         {
    //             label[label_length] = '\0';
    //             break;
    //         }
    //         label[label_length] = decoded_data[label_length];
    //     }

    //     if (strcmp(label, find_label) == 0)
    //     {
    //         memcpy(decoded_data, label, label_length);
    //         decoded_data[label_length] = ' ';
    //         memcpy(decoded_data + label_length + 1, data, data_length);

    //         AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    //         AES_CTR_xcrypt_buffer(&ctx, (uint8_t *)decoded_data,
    //                               label_and_data_size);

    //         size_t encsize;
    //         char *encoded_data = b64_encode(decoded_data, label_and_data_size, &encsize);

    //         FILE *f = fopen(data_store, "wb");
    //         if (f == NULL)
    //             PANIC_OPEN_FILE(data_store);

    //         memset(lines[i], 0, line_length);
    //         {
    //             void *tmp = realloc(lines[i], encsize + 1);
    //             if (tmp == NULL)
    //                 PANIC("%s\n", "memory allocation failed!");
    //             lines[i] = (char *)tmp;
    //         }
    //         strcpy(lines[i], encoded_data);

    //         for (size_t k = 0; k < idx; k++)
    //         {
    //             fprintf(f, "%s\n", (char *)lines[k]);
    //             free(lines[k]);
    //         }

    //         fclose(f);
    //         free(lines);

    //         upload_changes(sync_remote_url);
    //         return;
    //     }
    // }

    // uint8_t *label_and_data = (uint8_t *)malloc(label_and_data_size);
    // memcpy(label_and_data, find_label, find_label_length);
    // label_and_data[find_label_length] = ' ';
    // memcpy(label_and_data + find_label_length + 1, data, data_length);

    // AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    // AES_CTR_xcrypt_buffer(&ctx, label_and_data, label_and_data_size);

    // size_t encsize;
    // char *encoded_data = b64_encode(label_and_data, label_and_data_size, &encsize);
    // write_file(data_store, "a", encoded_data, encsize);

    // for (size_t i = 0; i < idx; i++)
    //     free(lines[i]);

    // free(lines);

    // upload_changes(sync_remote_url);
}

void encrypt_and_write(Flags *fl, String s, uint8_t *aes_key)
{
    if (!fl->binary.exists)
        pull_changes(sync_remote_url);

    File f = create_file(data_store, PM_READ_WRITE);
    input_key(aes_key, fl);
    xcrypt_buffer(s.data, aes_key, s.length);

    if (fl->binary.exists)
    {
        TRUNCATE_FILE_OR_EXIT(&f, s.length);
        MAP_FILE_OR_EXIT(&f);
        memcpy(f.start, s.data, s.length);
        unmap_and_close_file(f);
    }
    else
    {
        size_t b64_encoded_len;
        char *b64_encoded_str = b64_encode(s.data, s.length, &b64_encoded_len);
        size_t initial_size = f.size;
        TRUNCATE_FILE_OR_EXIT(&f, f.size + b64_encoded_len + 1);
        MAP_FILE_OR_EXIT(&f);
        memcpy(f.start + initial_size, b64_encoded_str, b64_encoded_len);
        free(b64_encoded_str);
        f.start[initial_size + b64_encoded_len] = '\n';
        unmap_and_close_file(f);
        upload_changes(sync_remote_url);
    }
}

void delete_label(Flags *fl, String label, uint8_t *aes_key)
{
//     File f = open_and_map_file(data_store, PM_READ_WRITE);
//     input_key(aes_key, fl);

//     size_t i = 0;
//     size_t p = 0;
//     do
//     {
//         if (f.start[p] == '\n')
//         {
//             size_t b64_decoded_len;
//             uint8_t *b64_decoded_str = b64_decode(f.start + i, p - i, &b64_decoded_len);
//             xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
//             if ((label.length + 1 >= b64_decoded_len) ||
//                 (memcmp(b64_decoded_str, fl->find_label.value, label.length) != 0))
//                 goto skip_line;
//             size_t label_len = 0;
//             while (b64_decoded_str[++label_len] != ' ')
//                 if (label_len > b64_decoded_len)
//                     goto skip_line;
            
// skip_line:
//             i = p + 1;
//             free(b64_decoded_str);
//         }
//         p++;
//     } while (p < f.size);

//     info("%s", "no results");
//     unmap_and_close_file(f);
//     upload_changes(sync_remote_url);
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

    File f = open_and_map_file(data_store, PM_READ_ONLY);
    input_key(aes_key, fl);

    int found_label = 0;
    if (fl->binary.exists)
    {
        uint8_t *file_copy = calloc(1, f.size);
        ASSERT_NOT_NULL(file_copy);
        memcpy(file_copy, f.start, f.size);
        xcrypt_buffer(file_copy, aes_key, f.size);
        if (fwrite(file_copy, 1, f.size, o) != f.size)
            error("%s", "fwrite failed");
        fflush(o);
        free(file_copy);
        goto end;
    }

    size_t find_label_len = 0;
    if (fl->find_label.exists)
        find_label_len = strlen(fl->find_label.value);

    size_t i = 0;
    size_t p = 0;
    do
    {
        if (f.start[p] == '\n')
        {
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + i, p - i, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            if (fl->find_label.exists)
            {
                if ((find_label_len + 1 >= b64_decoded_len) ||
                    (memcmp(b64_decoded_str, fl->find_label.value, find_label_len) != 0))
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
            i = p + 1;
            free(b64_decoded_str);
        }
        p++;
    } while (p < f.size);
    fflush(o);
end:
    if (fl->find_label.exists && !found_label)
        info("%s", "no results");
    if (fl->output.exists)
        fclose(o);
    unmap_and_close_file(f);
}
