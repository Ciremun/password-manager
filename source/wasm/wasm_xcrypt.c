// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "rawdraw/rd_util.h"
#include "rawdraw/rd_ui.h"
#include "wasm/wasm_xcrypt.h"

void decrypt_and_draw(uint8_t *aes_key)
{
    char *str = calloc(1, sizeof("tsodinSleep"));
    memcpy(str, "tsodinSleep", sizeof("tsodinSleep") - 1);
    append_input_field(create_input_field(RD_STR(str, sizeof("tsodinSleep") - 1)));
}