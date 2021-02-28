#include "io/common.h"
#include "rand.h"

extern struct AES_ctx ctx;
extern const char *help_s;

int is_flag(char *arg, char *s, char *l)
{
    return (strcmp(s, arg) == 0) || (strcmp(l, arg) == 0);
}

// TODO(#18): key flag
// argv on Windows has garbage
// argv key string differs from Unix getpasswd() input

void parse_flags(Flags *f, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        Flag *flag = NULL;
        if      (!f->data.exists               && is_flag(argv[i], "-d", "--data"))
            flag = &f->data;
        else if (!f->label.exists              && is_flag(argv[i], "-l", "--label"))
            flag = &f->label;
        else if (!f->find_label.exists         && is_flag(argv[i], "-fl", "--find-label"))
            flag = &f->find_label;
        else if (!f->help.exists               && is_flag(argv[i], "-h", "--help"))
            flag = &f->help;
        else if (!f->data_file.exists          && is_flag(argv[i], "-df", "--data-file"))
            flag = &f->data_file;
        else if (!f->generate_password.exists  && is_flag(argv[i], "-gp", "--generate-password"))
            flag = &f->generate_password;

        if (flag != NULL)
        {
            flag->exists = 1;
            if ((i + 1) >= argc)
            {
                continue;
            }
            flag->value = argv[i + 1];
        }
    }
}

int run(uint8_t *aes_key, int argc, char **argv)
{
    Flags f = {0};

    parse_flags(&f, argc, argv);

    if (!f.data.exists)
    {
        if (f.data_file.exists)
        {

            if (f.generate_password.exists)
            {
                printf("error: can't combine data-file and generate-password flags\n");
                return 1;
            }

            if (!f.data_file.value)
            {
                printf("error: data-file flag called without filename\n");
                return 1;
            }

            size_t nch = 0;
            char *data = read_file_as_str(f.data_file.value, &nch);

            if (f.label.exists)
            {
                encrypt_and_replace(f.label.value, data, aes_key);
            }
            else
            {
                encrypt_and_write((uint8_t *)data, aes_key, nch);
            }

            free(data);
            return 0;
        }
        if (f.generate_password.exists)
        {
            int password_length = random_int();
            char *password = malloc(password_length + 1);
            random_string(password_length, password);

            if (f.label.exists)
            {
                encrypt_and_replace(f.label.value, password, aes_key);
            }
            else
            {
                encrypt_and_write((uint8_t *)password, aes_key, password_length + 1);
            }

            free(password);
            return 0;
        }
        if (f.label.exists)
        {
            printf("error: label flag called without --data or --data-file or --generate-password\n");
            return 1;
        }
        if (f.find_label.exists)
        {
            if (!f.find_label.value)
            {
                printf("error: find label flag called without name\n");
                return 1;
            }
            decrypt_and_print(aes_key, f.find_label.value);
        }
        else
        {
            if (f.help.exists)
            {
                printf("%s\n", help_s);
                return 0;
            }

            decrypt_and_print(aes_key, NULL);
        }
    }

    if (!f.data.value)
    {
        printf("error: data flag called without data\n");
        return 1;
    }

    if (f.data_file.exists)
    {
        printf("error: can't combine data and data-file flags\n");
        return 1;
    }

    if (f.generate_password.exists)
    {
        printf("error: can't combine data and generate-password flags\n");
        return 1;
    }

    if (f.label.exists)
    {
        if (!f.label.value)
        {
            printf("error: label flag called without name\n");
            return 1;
        }
        encrypt_and_replace(f.label.value, f.data.value, aes_key);
    }
    else
    {
        encrypt_and_write((uint8_t *)f.data.value, aes_key, strlen(f.data.value) + 1);
    }

    return 0;
}
