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

typedef struct
{
    char *start;
    char *base;
} Memory;

int   mem_init(Memory *memory);
int   mem_free(Memory *memory);
void *mem_alloc(Memory *memory, u64 size);

#endif // MEM_H_

#ifdef MEM_IMPLEMENTATION

int mem_init(Memory *memory)
{
#ifdef _WIN32
    u64 reserved;
    if (!GetPhysicallyInstalledSystemMemory(&reserved))
        reserved = (u64)4294967295;
    else
        reserved *= 1024;
    memory->base
        = (char *)(VirtualAlloc(0, reserved, MEM_RESERVE, PAGE_NOACCESS));
    if (memory->base == 0)
        return 0;
#else
    memory->base = (char *)(mmap(0, 1, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (memory->base == MAP_FAILED)
        return 0;
#endif // _WIN32
    memory->start = memory->base;
    return 1;
}

int mem_free(Memory *memory)
{
    if (memory->start - memory->base == 0)
        return 1;
#ifdef _WIN32
    if (VirtualFree(memory->base, 0, MEM_RELEASE) == 0)
        return 0;
#else
    if (munmap(memory->base, memory->start - memory->base) == -1)
        return 0;
#endif // _WIN32
    return 1;
}

void *mem_alloc(Memory *memory, u64 size)
{
#ifdef _WIN32
    if (VirtualAlloc(memory->start, size, MEM_COMMIT, PAGE_READWRITE) == 0)
        return 0;
#else
    if (mmap(memory->start, size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
        == MAP_FAILED)
        return 0;
#endif // _WIN32
    memory->start += size;
    return memory->start - size;
}

#endif // MEM_IMPLEMENTATION
