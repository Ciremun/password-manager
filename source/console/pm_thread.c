// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "console/thread.h"

thread_load_info calc_thread_count_and_load(size_t size)
{
    int thread_count = 4;
    size_t load = size / thread_count;
    size_t remainder = size % thread_count;
    thread_load_info info;
    info.load = load;
    info.remainder = remainder;
    info.thread_count = thread_count;
    return info;
}

#ifdef _WIN32
og_thread_t OGCreateThread(void *(routine)(void *), void *parameter)
{
    return (og_thread_t)CreateThread(0, 0, (LPTHREAD_START_ROUTINE)routine, parameter, 0, 0);
}

void *OGJoinThread(og_thread_t ot)
{
    WaitForSingleObject(ot, INFINITE);
    CloseHandle(ot);
    return 0;
}
#else
og_thread_t OGCreateThread(void *(routine)(void *), void *parameter)
{
    pthread_t *ret = (pthread_t *)malloc(sizeof(pthread_t));
    if (!ret)
        return 0;
    int r = pthread_create(ret, 0, routine, parameter);
    if (r)
    {
        free(ret);
        return 0;
    }
    return (og_thread_t)ret;
}

void *OGJoinThread(og_thread_t ot)
{
    void *retval;
    if (!ot)
    {
        return 0;
    }
    pthread_join(*(pthread_t *)ot, &retval);
    free(ot);
    return retval;
}
#endif // _WIN32