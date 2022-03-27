#ifndef PM_RAND_H_
#define PM_RAND_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int random_int();
void random_string(size_t sz, uint8_t *out);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PM_RAND_H_
