#ifndef PM_XCRYPT_H_
#define PM_XCRYPT_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdio.h>
#endif // _WIN32

#include "pm_parse.h"
#include "pm_util.h"

void xcrypt_buffer(uint8_t *line, uint8_t *aes_key, size_t length);
void decrypt_and_print(Flags *fl, uint8_t *aes_key);
void encrypt_and_write(Flags *fl, String s, uint8_t *aes_key);
void encrypt_and_replace(Flags *fl, String s, String label, uint8_t *aes_key);
void delete_label(Flags *fl, String label, uint8_t *aes_key);

#endif // PM_XCRYPT_H_
