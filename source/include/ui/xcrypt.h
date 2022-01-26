#ifndef UI_XCRYPT_H_
#define UI_XCRYPT_H_

#include <stdint.h>

#include "core/util.h"

void ui_encrypt_and_write(String s, uint8_t *aes_key);
void ui_decrypt_and_draw(uint8_t *aes_key);

#endif // UI_XCRYPT_H_
