#include <limits.h>

#include "common.h"
#include "rand.h"
#include "version.h"

extern struct AES_ctx ctx;
extern const char *help_s;
extern char* data_store;

char* sync_remote_url = 0;

int is_flag(char *arg, char *s, char *l)
{
    return (strcmp(s, arg) == 0) || (strcmp(l, arg) == 0);
}

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
        else if (!f->key.exists                && is_flag(argv[i], "-k", "--key"))
            flag = &f->key;
        else if (!f->copy.exists               && is_flag(argv[i], "-c", "--copy"))
            flag = &f->copy;
        else if (!f->delete_label.exists       && is_flag(argv[i], "-dl", "--delete-label"))
            flag = &f->delete_label;
        else if (!f->input.exists              && is_flag(argv[i], "-i", "--input"))
            flag = &f->input;
        else if (!f->key_file.exists           && is_flag(argv[i], "-kf", "--key-file"))
            flag = &f->key_file;
        else if (!f->version.exists            && is_flag(argv[i], "-v", "--version"))
            flag = &f->version;

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
    sync_remote_url = getenv("PM_SYNC_REMOTE_URL");

    Flags f = {0};
    parse_flags(&f, argc, argv);

    if (f.version.exists)
    {
        printf("password-manager %s\n", PM_VERSION);
        return 0;
    }

    if (f.key.exists)
    {
        if (!f.key.value)
        {
            printf("error: key flag called without key value\n");
            return 1;
        }

        size_t aes_key_length = strlen(f.key.value) + 1;
        if (aes_key_length > 128)
        {
            aes_key = malloc(aes_key_length);
        }
        else
        {
            aes_key = calloc(1, 128);
        }
        memcpy(aes_key, f.key.value, aes_key_length);
    }

    if (f.key_file.exists)
    {
        if (!f.key_file.value)
        {
            printf("error: key-file flag called without filename\n");
            return 1;
        }
        size_t aes_key_length = 0;
        char *key_file = read_file_as_str(f.key_file.value, &aes_key_length);
        if (aes_key_length > 128)
        {
            aes_key = malloc(aes_key_length);
        }
        else
        {
            aes_key = calloc(1, 128);
        }
        memcpy(aes_key, key_file, aes_key_length);
        free(key_file);
    }

    if (f.input.exists)
    {
        if (f.input.value)
        {
            data_store = f.input.value;
        }
        else
        {
            printf("error: input flag called without file path\n");
            return 1;
        }
    }
    else
    {
        data_store = DEFAULT_DATA_STORE;
    }

    if (f.delete_label.exists)
    {
        if (f.delete_label.value)
        {
            delete_label(f.delete_label.value, aes_key);
            return 0;
        }
        else
        {
            printf("error: delete-label flag called without label\n");
            return 1;
        }
    }

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
                encrypt_and_replace(&f, f.label.value, data, aes_key);
            }
            else
            {
                encrypt_and_write(&f, (uint8_t *)data, aes_key, nch);
            }

            free(data);
            return 0;
        }
        if (f.generate_password.exists)
        {
            unsigned long password_length;
            if (f.generate_password.value)
            {
                password_length = strtoul(f.generate_password.value, NULL, 10);
                if (password_length == 0)
                {
                    password_length = (unsigned long)random_int();
                }
                else if (!(password_length <= INT_MAX))
                {
                    printf("error: length %s is out of range 1-%d\n", f.generate_password.value, INT_MAX);
                    return 1;
                }
            }
            else
            {
                password_length = (unsigned long)random_int();
            }
            char *password = malloc(password_length + 1);
            random_string((int)password_length, password);
            if (f.label.exists)
            {
                encrypt_and_replace(&f, f.label.value, password, aes_key);
            }
            else
            {
                encrypt_and_write(&f, (uint8_t *)password, aes_key, (int)password_length + 1);
            }
            if (f.copy.exists)
            {
#ifdef _WIN32
                copy_to_clipboard(password, password_length + 1);
#else
                printf("%s", password);
#endif
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
            decrypt_and_print(aes_key, &f);
        }
        else
        {
            if (f.help.exists)
            {
                printf("%s\n", help_s);
                return 0;
            }

            decrypt_and_print(aes_key, &f);
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
        encrypt_and_replace(&f, f.label.value, f.data.value, aes_key);
    }
    else
    {
        encrypt_and_write(&f, (uint8_t *)f.data.value, aes_key, strlen(f.data.value) + 1);
    }

    return 0;
}
