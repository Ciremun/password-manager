// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "core/aes.h"
#include "core/b64.h"
#include "core/io.h"
#include "core/sync.h"
#include "core/util.h"
#include "core/xcrypt.h"
#include "rawdraw/rd_xcrypt.h"
#include "rawdraw/rd_ui.h"

extern String sync_remote_url;
extern char *data_store;

void rd_encrypt_and_write(String s, uint8_t *aes_key)
{
    File f = create_file(data_store, PM_READ_WRITE);
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

void decrypt_and_draw(uint8_t *aes_key)
{
    pull_changes(sync_remote_url);
    File f = open_file(data_store, PM_READ_ONLY);

    if (f.size == 0)
    {
        error("file %s is empty", data_store);
        CLOSE_FILE(f.handle);
        return;
    }
    else
        MAP_FILE_(&f);

    // input_key(aes_key);

    int found_label = 0;
    // if (fl->binary.exists)
    // {
    //     uint8_t *file_copy = calloc(1, f.size);
    //     ASSERT_ALLOC(file_copy);
    //     memcpy(file_copy, f.start, f.size);
    //     xcrypt_buffer(file_copy, aes_key, f.size);
    //     // output to screen here
    //     free(file_copy);
    //     goto end;
    // }

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (f.start[line_end] == '\n')
        {
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + line_start, line_end - line_start, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            // if (fl->label.exists)
            // {
            //     if ((find_label_len + 1 >= b64_decoded_len) ||
            //         (memcmp(b64_decoded_str, fl->label.value, find_label_len) != 0))
            //         goto skip_write;
            //     size_t label_len = 0;
            //     while (b64_decoded_str[++label_len] != ' ')
            //         if (label_len > b64_decoded_len)
            //             goto skip_write;
            //     found_label = 1;
            //     if (fl->copy.exists)
            //     {
            //         if (!copy_to_clipboard(b64_decoded_str + label_len + 1, b64_decoded_len - label_len))
            //             error("%s", "couldn't copy to clipboard");
            //         free(b64_decoded_str);
            //         goto end;
            //     }
            // }

            // append_input_field(create_input_field(RD_STR(b64_decoded_str, b64_decoded_len)));
        skip_write:
            line_start = line_end + 1;
        }
        line_end++;
    } while (line_end < f.size);
end:
    // if (fl->label.exists && !found_label)
    //     info("%s", "no results");
    UNMAP_AND_CLOSE_FILE(f);
}