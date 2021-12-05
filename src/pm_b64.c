#include "pm_b64.h"
#include "pm_util.h"

#include <ctype.h>
#include <stdlib.h>

uint8_t *b64_decode(const uint8_t *src, size_t len, size_t *out_len)
{
    int i = 0;
    int j = 0;
    int l = 0;
    size_t ln = 0;
    uint8_t *dec = 0;
    uint8_t buf[3];
    uint8_t tmp[4];

    dec = (uint8_t *)calloc(1, len * 3 / 4 + 1);
    ASSERT_ALLOC(dec);

    while (len--)
    {
        if ('=' == src[j])
        {
            break;
        }
        if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j]))
        {
            break;
        }

        tmp[i++] = src[j++];

        if (4 == i)
        {
            for (i = 0; i < 4; ++i)
            {
                for (l = 0; l < 64; ++l)
                {
                    if (tmp[i] == b64_table[l])
                    {
                        tmp[i] = l;
                        break;
                    }
                }
            }

            buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
            buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
            buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

            for (i = 0; i < 3; ++i)
            {
                dec[ln++] = buf[i];
            }

            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 4; ++j)
        {
            tmp[j] = '\0';
        }

        for (j = 0; j < 4; ++j)
        {
            for (l = 0; l < 64; ++l)
            {
                if (tmp[j] == b64_table[l])
                {
                    tmp[j] = l;
                    break;
                }
            }
        }

        buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
        buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
        buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

        for (j = 0; (j < i - 1); ++j)
        {
            dec[ln++] = buf[j];
        }
    }

    dec[ln] = '\0';

    if (out_len != 0)
        *out_len = ln;

    return dec;
}

char *b64_encode(const uint8_t *src, size_t len, size_t *out_len)
{
    int i = 0;
    int j = 0;
    char *enc = 0;
    size_t ln = 0;
    uint8_t buf[4];
    uint8_t tmp[3];

    enc = (char *)calloc(1, (((4 * len / 3) + 3) & ~3) + 1);
    ASSERT_ALLOC(enc);

    while (len--)
    {
        tmp[i++] = *(src++);

        if (3 == i)
        {
            buf[0] = (tmp[0] & 0xfc) >> 2;
            buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
            buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
            buf[3] = tmp[2] & 0x3f;

            for (i = 0; i < 4; ++i)
            {
                enc[ln++] = b64_table[buf[i]];
            }

            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 3; ++j)
        {
            tmp[j] = '\0';
        }

        buf[0] = (tmp[0] & 0xfc) >> 2;
        buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
        buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
        buf[3] = tmp[2] & 0x3f;

        for (j = 0; (j < i + 1); ++j)
        {
            enc[ln++] = b64_table[buf[j]];
        }

        while ((i++ < 3))
        {
            enc[ln++] = '=';
        }
    }

    enc[ln] = '\0';

    if (out_len != 0)
        *out_len = ln;

    return enc;
}
