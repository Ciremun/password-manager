#ifndef PM_PARSE_H_
#define PM_PARSE_H_

#include <limits.h>
#include <stdlib.h>

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
    Flag find_label;
    Flag delete_label;
    Flag copy;
    Flag generate_password;
    Flag key;
    Flag key_file;
    Flag input;
    Flag output;
    Flag binary;
    Flag version;
    Flag help;
} Flags;

int is_flag(char *arg, char *s, char *l);
void parse_flags(Flags *f, int argc, char **argv);
int run(uint8_t *aes_key, int argc, char **argv);

#endif // PM_PARSE_H_
