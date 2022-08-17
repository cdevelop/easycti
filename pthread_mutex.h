#ifndef PTHREAD_MUTEX_H_
#define PTHREAD_MUTEX_H_

#include <pthread.h>


class PthreadMutex
{
private:
    PthreadMutex(const PthreadMutex&)
    {

    }
    PthreadMutex& operator= (const PthreadMutex&)
    {
        return *this;
    }
public:
    PthreadMutex(bool recursive=true)
    {
        pthread_mutexattr_init(&_mutexatt);
        if(recursive)
            pthread_mutexattr_settype(&_mutexatt,PTHREAD_MUTEX_RECURSIVE);
        else
            pthread_mutexattr_settype(&_mutexatt,PTHREAD_MUTEX_NORMAL);

        pthread_mutex_init(&_mutex,&_mutexatt);
    }
    ~PthreadMutex(void)
    {
        pthread_mutex_destroy(&_mutex);
        pthread_mutexattr_destroy(&_mutexatt);
    }
    void Lock()
    {
        pthread_mutex_lock(&_mutex);
    }
    void UnLock()
    {
        pthread_mutex_unlock(&_mutex);
    }
    bool TryLock()
    {
        return pthread_mutex_trylock(&_mutex)==0;
    }
    pthread_mutex_t&  mutex()
    {
        return _mutex;
    }

private:
    pthread_mutex_t        _mutex;
    pthread_mutexattr_t    _mutexatt;
};

class PthreadMutexScope
{
public:
    PthreadMutexScope(PthreadMutex &mutex):_mutex(mutex)
    {
        _mutex.Lock();
    }
    ~PthreadMutexScope()
    {
        _mutex.UnLock();
    }
private:
    PthreadMutex    &_mutex;
};


#endif //PTHREAD_MUTEX_H_