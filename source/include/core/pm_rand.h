#ifndef PM_RAND_H_
#define PM_RAND_H_

#include <stddef.h>
#include <stdint.h>

int random_int();
void random_string(size_t sz, uint8_t *out);

#endif // PM_RAND_H_