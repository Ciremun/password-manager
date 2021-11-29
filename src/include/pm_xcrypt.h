#ifndef PM_XCRYPT_H_
#define PM_XCRYPT_H_

#include "pm_parse.h"
#include "pm_util.h"

#ifdef _WIN32
int copy_to_clipboard(const char *password, size_t size);
#endif

uint8_t *decrypt_base64(String line, uint8_t *aes_key, size_t *decoded_line_length);
void xcrypt_buffer(uint8_t *line, uint8_t *aes_key, size_t length);

void decrypt_and_print(Flags *f, uint8_t *aes_key);
void encrypt_and_write(Flags *f, String data, uint8_t *aes_key);
void encrypt_and_replace(Flags *f, String data, uint8_t *aes_key, char *label);
void delete_label(char *label, uint8_t *aes_key);

#endif // PM_XCRYPT_H_
