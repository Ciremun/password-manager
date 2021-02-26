#include "common.h"

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF)
    {
        return -1;
    }

    if (*lineptr == NULL)
    {
        *lineptr = malloc(128);
        if (*lineptr == NULL)
        {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while (c != EOF)
    {
        if (pos + 1 >= *n)
        {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128)
            {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL)
            {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos++] = c;
        if (c == '\n')
        {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

void read_file(const char *fp, char ***lines, size_t *lsize)
{
    char *ln = NULL;
    size_t n = 0;
    ssize_t nchr = 0;
    size_t idx = 0;
    size_t lmax = LMAX;
    FILE *f = NULL;

    if (!(f = fopen(fp, "r")))
    {
        printf("error opening file %s\n", fp);
        exit(1);
    }

    if (!(*lines = calloc(LMAX, sizeof(**lines))))
    {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(1);
    }

    while ((nchr = getline(&ln, &n, f)) != -1)
    {
        while (nchr > 0 && (ln[nchr - 1] == '\n' || ln[nchr - 1] == '\r'))
            ln[--nchr] = 0;

        (*lines)[idx++] = strdup(ln);

        if (idx == lmax)
        {
            char **tmp = realloc(lines, lmax * 2 * sizeof *lines);
            if (!tmp)
            {
                fprintf(stderr, "error: memory allocation failed\n");
                exit(1);
            }
            *lines = tmp;
            lmax *= 2;
        }
    }

    if (f)
        fclose(f);
    if (ln)
        free(ln);

    *lsize = idx;
}

char *read_file_as_str(const char *fp, size_t *nch)
{
    FILE *f = fopen(fp, "r");
    if (f == NULL)
    {
        printf("error opening file %s\n", fp);
        exit(1);
    }
    int c;
    size_t size = 1024;
    char *buf = malloc(size);
    if (buf == NULL)
    {
        fprintf(stderr, "error: memory allocation failed\n");
        exit(1);
    }

    while ((c = getc(f)) != EOF)
    {
        if (*nch >= size - 1)
        {
            size *= 2;
            buf = realloc(buf, size);
            if (buf == NULL)
            {
                fprintf(stderr, "error: memory allocation failed\n");
                exit(1);
            }
        }
        buf[(*nch)++] = c;
    }

    buf[(*nch)++] = 0;
    fclose(f);
    return buf;
}

void write_file(const char *fp, const char *mode, void *data)
{
    FILE *f = fopen(fp, mode);
    if (f == NULL)
    {
        printf("Error opening file %s\n", fp);
        exit(1);
    }
    fprintf(f, "%s\n", (char *)data);
    fclose(f);
}
