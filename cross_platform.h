/*!
 * \file cross_platform.h
 *
 * \author xu
 * \date 2015/10/28
 * \brief 常用函数的跨平台实现
 * 
 */

#ifndef CROSS_PLATFORM_H_
#define CROSS_PLATFORM_H_


#ifdef _WIN32


#include <windows.h>


#define cross_sleep(ms)  Sleep(ms)

#if _MSC_VER<1900 
#define snprintf   _snprintf 
#endif



static unsigned long long GetTickMs()
{
#ifndef WINMM
#if (_WIN32_WINNT>=0x0502)
    return GetTickCount64();
#else
    return GetTickCount();
#endif
#else
    return timeGetTime();
#endif
}


static unsigned long long GetTickUs()
{

    static LARGE_INTEGER Frequency = {0};
    
    if (Frequency.QuadPart == 0){
        QueryPerformanceFrequency(&Frequency);
    }

    LARGE_INTEGER StartingTime;
    QueryPerformanceCounter(&StartingTime);

  

    StartingTime.QuadPart *= 1000000;
    StartingTime.QuadPart /= Frequency.QuadPart;

    return  StartingTime.QuadPart;
}






#else

#include <unistd.h>
#include <sys/types.h>  
#include <time.h>
#define  cross_sleep(msec)  usleep(msec*1000)

static unsigned long long GetTickMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


static unsigned long long GetTickUs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000);
}

#endif





#endif //CROSS_PLATFORM_H_