#include <limits.h>

#include "common.h"
#include "rand.h"
#include "version.h"

#define MEM_IMPLEMENTATION
#include "mem.h"

Memory g_mem;
extern struct AES_ctx ctx;
extern const char    *help_s;
extern char          *data_store;

char *sync_remote_url = 0;

int is_flag(char *arg, char *s, char *l)
{
    return (strcmp(s, arg) == 0) || (strcmp(l, arg) == 0);
}

void parse_flags(Flags *f, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        Flag *flag = NULL;
        if (!f->data.exists && is_flag(argv[i], "-d", "--data"))
            flag = &f->data;
        else if (!f->label.exists && is_flag(argv[i], "-l", "--label"))
            flag = &f->label;
        else if (!f->find_label.exists
                 && is_flag(argv[i], "-fl", "--find-label"))
            flag = &f->find_label;
        else if (!f->help.exists && is_flag(argv[i], "-h", "--help"))
            flag = &f->help;
        else if (!f->data_file.exists && is_flag(argv[i], "-df", "--data-file"))
            flag = &f->data_file;
        else if (!f->generate_password.exists
                 && is_flag(argv[i], "-gp", "--generate-password"))
            flag = &f->generate_password;
        else if (!f->key.exists && is_flag(argv[i], "-k", "--key"))
            flag = &f->key;
        else if (!f->copy.exists && is_flag(argv[i], "-c", "--copy"))
            flag = &f->copy;
        else if (!f->delete_label.exists
                 && is_flag(argv[i], "-dl", "--delete-label"))
            flag = &f->delete_label;
        else if (!f->input.exists && is_flag(argv[i], "-i", "--input"))
            flag = &f->input;
        else if (!f->key_file.exists && is_flag(argv[i], "-kf", "--key-file"))
            flag = &f->key_file;
        else if (!f->version.exists && is_flag(argv[i], "-v", "--version"))
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
    if (!mem_init(&g_mem))
        PANIC("%s\n", "mem_init failed!");

    sync_remote_url = getenv("PM_SYNC_REMOTE_URL");

    Flags f = {0};
    parse_flags(&f, argc, argv);

    if (f.version.exists)
    {
        fprintf(stdout, "%s %s\n", "password-manager", PM_VERSION);
        return 0;
    }

    if (f.key.exists)
    {
        if (!f.key.value)
        {
            error("%s\n", "key flag called without key value");
            return 1;
        }

        size_t aes_key_length = strlen(f.key.value) + 1;
        if (aes_key_length > 128)
            aes_key = mem_alloc(&g_mem, aes_key_length);
        else
            aes_key = mem_alloc(&g_mem, 128);
        if (aes_key == NULL)
            PANIC_MALLOC();
        memcpy(aes_key, f.key.value, aes_key_length);
    }

    if (f.key_file.exists)
    {
        if (!f.key_file.value)
        {
            error("%s\n", "key-file flag called without filename");
            return 1;
        }
        size_t aes_key_length = 0;
        char  *key_file = read_file_as_str(f.key_file.value, &aes_key_length);
        if (aes_key_length > 128)
            aes_key = malloc(aes_key_length);
        else
            aes_key = calloc(1, 128);
        if (aes_key == NULL)
            PANIC_MALLOC();
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
            error("%s\n", "input flag called without file path");
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
            error("%s\n", "delete-label flag called without label");
            return 1;
        }
    }

    if (!f.data.exists)
    {
        if (f.data_file.exists)
        {

            if (f.generate_password.exists)
            {
                error("%s\n", "can't combine data-file and "
                              "generate-password flags");
                return 1;
            }

            if (!f.data_file.value)
            {
                error("%s\n", "data-file flag called without filename");
                return 1;
            }

            size_t nch = 0;
            char  *data = read_file_as_str(f.data_file.value, &nch);

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
                    error("length %s is out of range 1-%d\n",
                          f.generate_password.value, INT_MAX);
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
                encrypt_and_write(&f, (uint8_t *)password, aes_key,
                                  (int)password_length + 1);
            }
            if (f.copy.exists)
            {
#ifdef _WIN32
                copy_to_clipboard(password, password_length + 1);
#else
                fprintf(stdout, "%s", password);
#endif
            }
            free(password);
            return 0;
        }
        if (f.label.exists)
        {
            error("%s\n", "label flag called without --data or --data-file or "
                          "--generate-password");
            return 1;
        }
        if (f.find_label.exists)
        {
            if (!f.find_label.value)
            {
                error("%s\n", "find label flag called without name");
                return 1;
            }
            decrypt_and_print(aes_key, &f);
        }
        else
        {
            if (f.help.exists)
            {
                fprintf(stdout, "%s\n", help_s);
                return 0;
            }

            decrypt_and_print(aes_key, &f);
        }
    }

    if (!f.data.value)
    {
        error("%s\n", "data flag called without data");
        return 1;
    }

    if (f.data_file.exists)
    {
        error("%s\n", "can't combine data and data-file flags");
        return 1;
    }

    if (f.generate_password.exists)
    {
        error("%s\n", "can't combine data and generate-password flags");
        return 1;
    }

    if (f.label.exists)
    {
        if (!f.label.value)
        {
            error("%s\n", "label flag called without name");
            return 1;
        }
        encrypt_and_replace(&f, f.label.value, f.data.value, aes_key);
    }
    else
    {
        encrypt_and_write(&f, (uint8_t *)f.data.value, aes_key,
                          strlen(f.data.value) + 1);
    }

    if (!mem_free(&g_mem))
        PANIC("%s\n", "mem_free failed!");

    return 0;
}
