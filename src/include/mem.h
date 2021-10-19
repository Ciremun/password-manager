#ifndef MEM_H_
#define MEM_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif // _WIN32

#include <string.h>

typedef unsigned long long int u64;

#ifndef _WIN32
int GetPhysicallyInstalledSystemMemory(u64 *output);
#endif // _WIN32

typedef struct
{
    char *start;
    char *base;
    u64 capacity;
#ifdef _WIN32
    u64 commited;
    u64 allocated;
    u64 commit_size;
#endif // _WIN32
} Memory;

int mem_init(Memory *memory);
int mem_free(Memory *memory);
void *mem_alloc(Memory *memory, u64 size);

#endif // MEM_H_

#ifdef MEM_IMPLEMENTATION

#ifndef _WIN32
int GetPhysicallyInstalledSystemMemory(u64 *output)
{
    int fd = open("/proc/meminfo", O_RDONLY);
    if (fd == -1)
        return 0;

    char buff[64];
    int count = 0;
    if ((count = read(fd, buff, 64)) <= 0)
        goto error;

    if (memcmp(buff, "MemTotal:", 9) != 0)
        goto error;

    u64 pos;
    pos = 9;

    while (buff[++pos] == ' ');

    u64 i;
    i = 0;

    u64 digit;

    while ((buff + pos)[i] != ' ')
    {
        digit = (buff + pos)[i] - 48;
        if (digit > 9)
            goto error;
        *output = *output * 10 + digit;
        i++;
    }

    close(fd);
    return 1;
error:
    close(fd);
    return 0;
}
#endif // _WIN32

int mem_init(Memory *memory)
{
    memory->capacity = 0;
    if (!GetPhysicallyInstalledSystemMemory(&memory->capacity))
        memory->capacity = (u64)4294967295;
    else
        memory->capacity *= 1024;
#ifdef _WIN32
    memory->commited = 0;
    memory->allocated = 0;
    memory->commit_size = 1024;
    memory->base = (char *)(VirtualAlloc(0, memory->capacity, MEM_RESERVE,
                                         PAGE_READWRITE));
    if (memory->base == 0)
        return 0;
#else
#ifdef TEST
    memory->capacity = (u64)536870912;
#endif // TEST
    memory->base = (char *)(mmap(0, memory->capacity, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (memory->base == MAP_FAILED)
        return 0;
#endif // _WIN32
    memory->start = memory->base;
    return 1;
}

int mem_free(Memory *memory)
{
#ifdef _WIN32
    if (VirtualFree(memory->base, 0, MEM_RELEASE) == 0)
        return 0;
#else
    if (munmap(memory->base, memory->capacity) == -1)
        return 0;
#endif // _WIN32
    return 1;
}

void *mem_alloc(Memory *memory, u64 size)
{
#ifdef _WIN32
    memory->allocated += size;
    if (memory->allocated * 2 > memory->commited)
    {
        memory->commit_size += size * 2;
        if (VirtualAlloc(memory->start, memory->commit_size, MEM_COMMIT,
                         PAGE_READWRITE)
            == 0)
            return 0;
        memory->commited += memory->commit_size;
    }
#endif // _WIN32
    memory->start += size;
    return memory->start - size;
}

#endif // MEM_IMPLEMENTATION