/**
 * `b64.h' - b64
 *
 * copyright (c) 2014 joseph werle
 */

#ifndef PM_B64_H_
#define PM_B64_H_

#include <stddef.h>
#include <stdint.h>

static const char b64_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    char *b64_encode(const uint8_t *, size_t, size_t *);
    uint8_t *b64_decode(const uint8_t *, size_t, size_t *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PM_B64_H_
