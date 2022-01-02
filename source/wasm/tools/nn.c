// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <stdio.h>
#include <string.h>

int main(void)
{
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, stdin) != NULL)
    {
        size_t i = 0;
        while (line[i])
        {
            if (line[i] != '\n')
                fputc(line[i], stdout);
            i++;
        }
    }
    fputc('\n', stdout);
    return 0;
}
