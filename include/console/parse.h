#ifndef PM_PARSE_H_
#define PM_PARSE_H_

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct
{
    char *value;
    int exists;
} Flag;

typedef struct
{
    Flag data;
    Flag data_file;
    Flag label;
    Flag delete_label;
    Flag copy;
    Flag generate_password;
    Flag key;
    Flag key_file;
    Flag input;
    Flag output;
    Flag binary;
    Flag b64enc;
    Flag b64dec;
    Flag version;
    Flag help;
} Flags;

int is_flag(char *arg, char *s, char *l);
void parse_flags(Flags *f, int argc, char **argv);
int run(uint8_t *aes_key, int argc, char **argv);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PM_PARSE_H_
