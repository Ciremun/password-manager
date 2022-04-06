// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "console/aes.h"
#include "console/b64.h"
#include "console/io.h"
#include "console/sync.h"
#include "console/util.h"
#include "console/xcrypt.h"
#include "ui/xcrypt.hpp"

extern String sync_remote_url;
extern "C" char *data_store;

void ui_write_encrypted_passwords(std::string const &str)
{
    File f = create_file(data_store, PM_READ_WRITE);
    TRUNCATE_FILE(&f, str.length());
    if (str.empty())
    {
        CLOSE_FILE(f.handle);
        return;
    }
    MAP_FILE_(&f);
    memcpy(f.start, str.c_str(), str.length());
    UNMAP_AND_CLOSE_FILE(f);
}

void ui_load_passwords(uint8_t *aes_key, ImVector<std::string *> &passwords)
{
    File f = create_file(data_store, PM_READ_ONLY);

    if (f.size == 0)
    {
        CLOSE_FILE(f.handle);
        return;
    }
    else
        MAP_FILE_(&f);

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (f.start[line_end] == '\n')
        {
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(f.start + line_start, line_end - line_start, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);    
            std::string *decoded_password = new std::string((const char *)b64_decoded_str);
            passwords.push_back(decoded_password);
            free(b64_decoded_str);
            line_start = line_end + 1;
        }
        line_end++;
    } while (line_end < f.size);
    UNMAP_AND_CLOSE_FILE(f);
}
