#include "pm_parse.h"
#include "pm_io.h"
#include "pm_xcrypt.h"
#include "pm_util.h"
#include "pm_rand.h"

#include <string.h>

String sync_remote_url;
extern char *data_store;

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
        else if (!f->find_label.exists && is_flag(argv[i], "-fl", "--find-label"))
            flag = &f->find_label;
        else if (!f->help.exists && is_flag(argv[i], "-h", "--help"))
            flag = &f->help;
        else if (!f->data_file.exists && is_flag(argv[i], "-df", "--data-file"))
            flag = &f->data_file;
        else if (!f->generate_password.exists && is_flag(argv[i], "-gp", "--generate-password"))
            flag = &f->generate_password;
        else if (!f->key.exists && is_flag(argv[i], "-k", "--key"))
            flag = &f->key;
        else if (!f->copy.exists && is_flag(argv[i], "-c", "--copy"))
            flag = &f->copy;
        else if (!f->delete_label.exists && is_flag(argv[i], "-dl", "--delete-label"))
            flag = &f->delete_label;
        else if (!f->input.exists && is_flag(argv[i], "-i", "--input"))
            flag = &f->input;
        else if (!f->key_file.exists && is_flag(argv[i], "-kf", "--key-file"))
            flag = &f->key_file;
        else if (!f->version.exists && is_flag(argv[i], "-v", "--version"))
            flag = &f->version;
        else if (!f->output.exists && is_flag(argv[i], "-o", "--output"))
            flag = &f->output;
        else if (!f->binary.exists && is_flag(argv[i], "-b", "--binary"))
            flag = &f->binary;

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
    sync_remote_url = (String) {
        .data = (uint8_t *)getenv("PM_SYNC_REMOTE_URL"),
    };

    if (sync_remote_url.data != 0)
        sync_remote_url.length = strlen((char *)sync_remote_url.data);

    Flags f = {0};
    parse_flags(&f, argc, argv);

    if (f.version.exists)
    {
        fprintf(stdout, "%s %s\n", "password-manager",
#ifdef PM_VERSION
                STR(PM_VERSION)
#else
                "unknown"
#endif // PM_VERSION
        );
        return 0;
    }

    if (f.key.exists)
    {
        if (!f.key.value)
        {
            error("%s", "key flag called without key value");
            return 1;
        }

        size_t key_len = strlen(f.key.value);
        memcpy(aes_key, f.key.value, key_len < 32 ? key_len : 32);
    }

    if (f.key_file.exists)
    {
        if (!f.key_file.value)
        {
            error("%s", "key-file flag called without filename");
            return 1;
        }
        File kf = open_and_map_file(f.key_file.value, PM_READ_ONLY);
        memcpy(aes_key, kf.start, kf.size < 32 ? kf.size : 32);
        UNMAP_AND_CLOSE_FILE(kf);
    }

    if (f.input.exists)
    {
        if (f.input.value)
        {
            data_store = f.input.value;
        }
        else
        {
            error("%s", "input flag called without file path");
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
            delete_label(&f, PM_STR(f.delete_label.value), aes_key);
            return 0;
        }
        else
        {
            error("%s", "delete-label flag called without label");
            return 1;
        }
    }

    if (!f.data.exists)
    {
        if (f.data_file.exists)
        {

            if (f.generate_password.exists)
            {
                error("%s", "can't combine data-file and "
                              "generate-password flags");
                return 1;
            }

            if (!f.data_file.value)
            {
                error("%s", "data-file flag called without filename");
                return 1;
            }

            File file = open_and_map_file(f.data_file.value, PM_READ_ONLY);
            String s = {
                .data = calloc(1, file.size + 1),
                .length = file.size,
            };
            ASSERT_ALLOC(s.data);
            memcpy(s.data, file.start, file.size);

            if (f.label.exists)
                encrypt_and_replace(&f, s, PM_STR(f.label.value), aes_key);
            else
                encrypt_and_write(&f, s, aes_key);

            free(s.data);
            UNMAP_AND_CLOSE_FILE(file);
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
            uint8_t *password_data = (uint8_t *)malloc(password_length + 1);
            random_string(password_length, password_data);
            String password = {
                .data = password_data,
                .length = password_length,
            };
            if (f.label.exists)
                encrypt_and_replace(&f, password, PM_STR(f.label.value), aes_key);
            else
                encrypt_and_write(&f, password, aes_key);
            if (f.copy.exists)
                copy_to_clipboard(password.data, password_length + 1);
            free(password_data);
            return 0;
        }
        if (f.label.exists)
        {
            error("%s", "label flag called without --data or --data-file or "
                          "--generate-password");
            return 1;
        }
        if (f.find_label.exists)
        {
            if (!f.find_label.value)
            {
                error("%s", "find label flag called without name");
                return 1;
            }
            decrypt_and_print(&f, aes_key);
            return 0;
        }
        else
        {
            if (f.help.exists)
            {
                fprintf(stdout, "%s\n", "\n"
                                        "./pm [flags]                  read or write data\n"
                                        "\n"
                                        "sync:                         set PM_SYNC_REMOTE_URL env var\n"
                                        "\n"
                                        "flags:\n"
                                        "\n"
                                        "-d  --data                    data to encrypt\n"
                                        "-df --data-file               data to encrypt from file\n"
                                        "-l  --label                   add label for data\n"
                                        "-fl --find-label              find data by label\n"
                                        "-dl --delete-label            delete label and its data\n"
#ifdef _WIN32
                                        "-c  --copy                    -fl, -gp helper, copy to clipboard\n"
#else
                                        "-c  --copy                    -fl, -gp helper, pipe with clip tools\n"
#endif
                                        "-gp --generate-password [N]   put random data\n"
                                        "-k  --key                     key\n"
                                        "-kf --key-file                key file path\n"
                                        "-i  --input                   encrypted file path\n"
                                        "-o  --output                  decrypted file path\n"
                                        "-b  --binary                  binary mode\n"
                                        "-v  --version                 display version\n"
                                        "-h  --help                    display help\n\n");
                return 0;
            }
            if (f.copy.exists)
            {
                error("%s",
                      "copy is only supported along with -fl, -gp flags");
                return 1;
            }
            decrypt_and_print(&f, aes_key);
            return 0;
        }
    }

    if (!f.data.value)
    {
        error("%s", "data flag called without data");
        return 1;
    }

    if (f.data_file.exists)
    {
        error("%s", "can't combine data and data-file flags");
        return 1;
    }

    if (f.generate_password.exists)
    {
        error("%s", "can't combine data and generate-password flags");
        return 1;
    }

    if (f.label.exists)
    {
        if (!f.label.value)
        {
            error("%s", "label flag called without name");
            return 1;
        }
        encrypt_and_replace(&f, PM_STR(f.data.value), PM_STR(f.label.value), aes_key);
    }
    else
    {
        encrypt_and_write(&f, PM_STR(f.data.value), aes_key);
    }

    return 0;
}
