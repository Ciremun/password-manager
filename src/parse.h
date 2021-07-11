#ifndef PARSE_H
#define PARSE_H

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
    Flag generate_password;
    Flag key;
    Flag copy;
    Flag delete_label;
    Flag help;
} Flags;

int is_flag(char *arg, char *s, char *l);
void parse_flags(Flags *f, int argc, char **argv);
int run(uint8_t *aes_key, int argc, char **argv);

#endif // PARSE_H
