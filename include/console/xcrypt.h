#ifndef PM_XCRYPT_H_
#define PM_XCRYPT_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdio.h>
#endif // _WIN32

#include <stdint.h>

#include "console/parse.h"
#include "console/util.h"
#include "console/thread.h"
#include "console/io.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct
{
    File file;
    String str;
    uint8_t *aes_key;
} xcrypt_and_write_load_ctx;

typedef struct
{
    uint8_t *str;
    uint8_t *aes_key;
} xcrypt_load_ctx;

void xcrypt_buffer(uint8_t *line, uint8_t *aes_key, size_t length);
void decrypt_and_print(Flags *fl, uint8_t *aes_key);
void encrypt_and_write(Flags *fl, String s, uint8_t *aes_key);
void encrypt_and_replace(Flags *fl, String s, String label, uint8_t *aes_key);
void delete_label(Flags *fl, String label, uint8_t *aes_key);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PM_XCRYPT_H_
