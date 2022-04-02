#include <string>

#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>
#include "imgui.h"

#include "console/b64.h"
#include "console/xcrypt.h"
#include "ui/xcrypt.hpp"

EM_JS(void, js_write_encrypted_passwords, (uint8_t *c_str), {
    var str = UTF8ToString(c_str);
    localStorage.passwords = str;
});

EM_JS(uint8_t *, js_get_encrypted_passwords, (void), {
    var s = localStorage.passwords;
    var size = lengthBytesUTF8(s) + 1;
    var str = _malloc(size);
    stringToUTF8Array(s, HEAP8, str, size);
    return str;
});

void write_encrypted_passwords(uint8_t *c_str)
{
    js_write_encrypted_passwords(c_str);
}

uint8_t* get_encrypted_passwords()
{
    return js_get_encrypted_passwords();
}

void ui_write_encrypted_passwords(std::string const &str)
{
    js_write_encrypted_passwords((uint8_t *)str.c_str());
}

void ui_load_passwords(uint8_t *aes_key, ImVector<std::string *> &passwords)
{
    uint8_t *encrypted_passwords = get_encrypted_passwords();
    size_t encrypted_passwords_size = strlen((char *)encrypted_passwords) + 1;

    size_t line_start = 0;
    size_t line_end = 0;
    do
    {
        if (encrypted_passwords[line_end] == '\n')
        {
            size_t b64_decoded_len;
            uint8_t *b64_decoded_str = b64_decode(encrypted_passwords + line_start, line_end - line_start, &b64_decoded_len);
            xcrypt_buffer(b64_decoded_str, aes_key, b64_decoded_len);
            std::string *decoded_password = new std::string((const char *)b64_decoded_str);
            passwords.push_back(decoded_password);
            free(b64_decoded_str);
            line_start = line_end + 1;
        }
        line_end++;
    } while (line_end < encrypted_passwords_size);
    free(encrypted_passwords);
}
