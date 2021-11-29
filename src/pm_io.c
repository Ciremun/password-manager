#include "pm_io.h"

char *data_store = 0;

File open_or_create_file(const char *path, flag_t access, int create)
{
    File f = {0};
#ifdef _WIN32
    DWORD dwCreationDisposition;
    if (create && !file_exists(path))
        dwCreationDisposition = CREATE_NEW;
    else
        dwCreationDisposition = OPEN_EXISTING;

    DWORD dwDesiredAccess;
    switch (access)
    {
    case PM_READ_WRITE:
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        break;
    case PM_READ_ONLY:
        dwDesiredAccess = GENERIC_READ;
        break;
    default:
        exit(1);
    }

    f.handle = CreateFileA(path, dwDesiredAccess, 0, 0, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
    if (f.handle == INVALID_HANDLE_VALUE)
    {
        error("CreateFileA failed: %ld\n", GetLastError());
        return f;
    }
    if (dwCreationDisposition == OPEN_EXISTING && !get_file_size(&f))
        exit(1);
#else
    int flags;

    switch (access)
    {
    case PM_READ_WRITE:
        flags = O_RDWR;
        break;
    case PM_READ_ONLY:
        flags = O_RDONLY;
        break;
    default:
        exit(1);
    }

    if (create && !file_exists(path))
        flags |= O_CREAT;

    f.handle = open(path, flags, 0600);
    if (f.handle < 0)
    {
        error("open failed: %s\n", strerror(errno));
        return f;
    }
    if (!(flags & O_CREAT) && !get_file_size(&f))
        exit(1);
#endif // _WIN32
    return f;
}

int close_file(File f)
{
#ifdef _WIN32
    if (CloseHandle(f.handle) == 0)
    {
        error("CloseHandle failed: %ld\n", GetLastError());
        return 0;
    }
#else
    if (close(f.handle) < 0)
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
    if ((ftruncate(h, new_size)) < 0)
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

void map_file(File *f)
{
#ifdef _WIN32
    DWORD flProtect;
    switch (f->access)
    {
    case PM_READ_WRITE:
        flProtect = PAGE_READWRITE;
        break;
    case PM_READ_ONLY:
        flProtect = PAGE_READONLY;
        break;
    default:
        exit(1);
    }

    f->hMap = CreateFileMappingA(f->handle, 0, flProtect, 0, 0, 0);
    if (f->hMap == 0)
    {
        error("CreateFileMappingA failed: %ld\n", GetLastError());
        CloseHandle(f->handle);
        exit(1);
    }

    DWORD dwDesiredAccess;
    switch (f->access)
    {
    case PM_READ_WRITE:
        dwDesiredAccess = FILE_MAP_ALL_ACCESS;
        break;
    case PM_READ_ONLY:
        dwDesiredAccess = FILE_MAP_READ;
        break;
    default:
        exit(1);
    }

    f->start = (char *)MapViewOfFile(f->hMap, dwDesiredAccess, 0, 0, 0);
    if (f->start == 0)
    {
        error("MapViewOfFile failed: %ld\n", GetLastError());
        CloseHandle(f->hMap);
        CloseHandle(f->handle);
        exit(1);
    }
#else

    int prot;
    switch (f->access)
    {
    case PM_READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PM_READ_ONLY:
        prot = PROT_READ;
        break;
    default:
        exit(1);
    }

    if ((f->start = mmap(0, f->size, prot, MAP_SHARED,
                          f->handle, 0)) == (void *)-1)
    {
        error("mmap failed: %s\n", strerror(errno));
        exit(1);
    }
#endif // _WIN32
}

int unmap_file(File f)
{
#ifdef _WIN32
    if (UnmapViewOfFile(f.start) == 0)
    {
        error("UnmapViewOfFile failed: %ld\n", GetLastError());
        return 0;
    }
    if (CloseHandle(f.hMap) == 0)
    {
        error("CloseHandle failed: %ld\n", GetLastError());
        return 0;
    }
#else
    if (munmap(f.start, f.size) < 0)
    {
        error("munmap failed: %s\n", strerror(errno));
        return 0;
    }
#endif // _WIN32
    return 1;
}

int getpasswd(uint8_t *pw)
{
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
        if (c != PM_BACKSPACE_KEY)
            pw[idx++] = c;
        else if (idx > 0)
            pw[--idx] = 0;
        if (idx >= 32)
            break;
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

void input_key(uint8_t *aes_key, Flags *f)
{
    if (f == 0 || !f->copy.exists)
        fprintf(stdout, "%s\n", "key?");
    getpasswd(aes_key);
}
