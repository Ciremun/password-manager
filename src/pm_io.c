#include "pm_io.h"

char *data_store = 0;

File open_file(const char *path)
{
    File f;
#ifdef _WIN32
    DWORD dwCreationDisposition = file_exists(path) ? OPEN_EXISTING : CREATE_NEW;

    f.handle = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, 0, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
    if (f.handle == INVALID_HANDLE_VALUE)
    {
        error("CreateFileA failed: %ld\n", GetLastError());
        exit(1);
    }
    if (dwCreationDisposition == CREATE_NEW)
    {
        f.size = 0;
    }
    if (dwCreationDisposition == OPEN_EXISTING && !get_file_size(&f))
    {
        exit(1);
    }
#else
    int flags = O_RDWR;
    int exists = file_exists(path);

    if (!exists)
    {
        flags |= O_CREAT;
        f.size = 0;
    }

    f.handle = open(path, flags, 0600);
    if (f.handle < 0)
    {
        error("open failed: %s\n", strerror(errno));
        exit(1);
    }
    if (exists && !get_file_size(&f))
    {
        exit(1);
    }
#endif // _WIN32
    return f;
}

int close_file(handle_t h)
{
#ifdef _WIN32
    if (CloseHandle(h) == 0)
    {
        error("CloseHandle failed: %ld\n", GetLastError());
        return 0;
    }
#else
    if (close(h) == -1)
    {
        error("close failed: %s\n", strerror(errno));
        return 0;
    }
#endif // _WIN32
    return 1;
}

int file_exists(const char *path)
{
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributes(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    return access(path, F_OK) == 0;
#endif // _WIN32
}

int truncate_file(handle_t h, size_t new_size)
{
#ifdef _WIN32
    if (SetFilePointer(h, new_size, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        error("SetFilePointer failed: %ld\n", GetLastError());
        CloseHandle(h);
        return 0;
    }

    if (SetEndOfFile(h) == 0)
    {
        error("SetEndOfFile failed: %ld\n", GetLastError());
        CloseHandle(h);
        return 0;
    }
#else
    if ((ftruncate(h, new_size)) == -1)
    {
        error("ftruncate failed: %s\n", strerror(errno));
        return 0;
    }
#endif // _WIN32
    return 1;
}

int get_file_size(File *f)
{
#ifdef _WIN32
    LARGE_INTEGER lpFileSize;
    if (!GetFileSizeEx(f->handle, &lpFileSize))
    {
        error("GetFileSizeEx failed: %ld\n", GetLastError());
        return 0;
    }
    f->size = lpFileSize.QuadPart;
#else
    struct stat statbuf;
    if (fstat(f->handle, &statbuf) < 0)
    {
        error("fstat failed: %s\n", strerror(errno));
        return 0;
    }
    f->size = statbuf.st_size;
#endif // _WIN32
    return 1;
}

char *map_file(File f)
{
#ifdef _WIN32
    HANDLE hMap = CreateFileMappingA(f.handle, 0, PAGE_READWRITE, 0, 0, 0);
    if (!hMap)
    {
        error("CreateFileMappingA failed: %ld\n", GetLastError());
        CloseHandle(f.handle);
        return 0;
    }

    char *map_start = (char *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (map_start == 0)
    {
        error("MapViewOfFile failed: %ld\n", GetLastError());
        CloseHandle(hMap);
        CloseHandle(f.handle);
        return 0;
    }
#else
    char *map_start;
    if ((map_start = mmap(0, f.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                          f.handle, 0)) == (void *)-1)
    {
        error("mmap failed: %s\n", strerror(errno));
        return 0;
    }
#endif // _WIN32
    return map_start;
}

int unmap_file(char *map_start, size_t size)
{
#ifdef _WIN32
    (void)size;
    if (UnmapViewOfFile(map_start) == 0)
    {
        error("UnmapViewOfFile failed: %ld\n", GetLastError());
        return 0;
    }
#else
    if (munmap(map_start, size) == -1)
    {
        error("munmap failed: %s\n", strerror(errno));
        return 0;
    }
#endif // _WIN32
    return 1;
}

int getpasswd(uint8_t **pw)
{
    // 32 max passwd len
    int c = 0;
    size_t idx = 0;

#ifndef _WIN32
    struct termios old_kbd_mode;
    struct termios new_kbd_mode;

    if (tcgetattr(0, &old_kbd_mode))
    {
        error("%s tcgetattr failed\n", __func__);
        return 0;
    }
    memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

    new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
    new_kbd_mode.c_cc[VTIME] = 0;
    new_kbd_mode.c_cc[VMIN] = 1;
    if (tcsetattr(0, TCSANOW, &new_kbd_mode))
    {
        error("%s tcsetattr failed\n", __func__);
        return 0;
    }
#endif // _WIN32
    while (
#ifdef _WIN32
        (c = _getch()) != 13
#else
        (c = fgetc(stdin)) != '\n'
#endif // _WIN32
        && c != EOF)
    {
        if (c != 127)
        {
            (*pw)[idx++] = c;
        }
        else if (idx > 0)
        {
            (*pw)[--idx] = 0;
        }
    }

#ifndef _WIN32
    if (tcsetattr(0, TCSANOW, &old_kbd_mode))
    {
        error("%s tcsetattr failed\n", __func__);
        return 0;
    }
#endif // _WIN32

    return 1;
}
