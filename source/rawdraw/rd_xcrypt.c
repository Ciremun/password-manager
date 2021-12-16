#include "rawdraw/vendor/os_generic.h"

#include "rawdraw/rd_xcrypt.h"
#include "core/pm_aes.h"
#include "core/pm_b64.h"
#include "core/pm_io.h"
#include "core/pm_sync.h"
#include "core/pm_util.h"
#include "core/pm_xcrypt.h"
#include "rawdraw/rd_ui.h"

extern String sync_remote_url;

void decrypt_and_draw(uint8_t *aes_key)
{
    pull_changes(sync_remote_url);
    File f = open_file(DEFAULT_DATA_STORE, PM_READ_ONLY);

    if (f.size == 0)
    {
        error("file %s is empty", DEFAULT_DATA_STORE);
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

            append_input_field(create_input_field(RD_STR(b64_decoded_str, b64_decoded_len)));
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
