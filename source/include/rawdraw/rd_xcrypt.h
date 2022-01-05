#ifndef PM_RD_XCRYPT_H_
#define PM_RD_XCRYPT_H_

#include <stdint.h>

#include "core/pm_util.h"

void rd_encrypt_and_write(String s, uint8_t *aes_key);
void decrypt_and_draw(uint8_t *aes_key);

#endif // PM_RD_XCRYPT_H_
