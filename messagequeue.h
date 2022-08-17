#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "pthread.h"
#include <queue>

#ifdef _WIN32
#include <timeapi.h>
#pragma comment(lib,"Winmm.lib")
#else
#include <time.h>
#endif

#include <limits.h>


/*!
 * \class messagequeue
 *
 * \brief 简单的消息队列
 *
 * \author xu(微信：cdevelop)
 * \date 2015/10/28
 */


template <class T>
class messagequeue
{
private:
    messagequeue() {}
public:
    struct MSG
    {
        int      type;
        T        data;
        unsigned time;
    };
    typedef void(*MESSAGE_PROC)(MSG *msg, void *custom);

    messagequeue(MESSAGE_PROC msgfun, void *funcustom)
        : f(msgfun)
        , fp(funcustom)
        , quit(false)
    {
        MSG msg = { 0 };
        q.push(msg);
        pthread_mutex_init(&m, NULL);
        pthread_cond_init(&c, NULL);
        pthread_create(&p, NULL, thread_proc, this);
    }

    ~messagequeue()
    {
        quit = true;
        pthread_cond_signal(&c);
        pthread_join(p,NULL);
        pthread_mutex_destroy(&m);
        pthread_cond_destroy(&c);
    }
    void post(T data)
    {
        post(0, &data);
    }
    void post(T *data)
    {
        post(0, data);
    }
    void post(int type, T data)
    {
        post(type, &data);
    }
    void post(int type,T *data)
    {
        pthread_mutex_lock(&m);
        MSG msg = {type,*data,currentTime()};
        q.push(msg);
        pthread_cond_signal(&c);
        pthread_mutex_unlock(&m);
    }
    static void *thread_proc(void *param)
    {
        messagequeue<T> *pThis = (messagequeue<T>*)param;
        pThis->loop();
        return NULL;
    }
    


private:

    std::queue<MSG>     q;
    pthread_mutex_t     m;
    pthread_cond_t      c;
    pthread_t           p;
    MESSAGE_PROC        f;
    void*               fp;
    bool                quit;


    void loop()
    {
        for (;;)
        {
            pthread_mutex_lock(&m);
            q.pop();
            if (q.empty())
            {
                if(!quit)
                    pthread_cond_wait(&c, &m);
                if (quit)
                {
                    pthread_mutex_unlock(&m);
                    break;
                }
            }
            MSG &msg = q.front();
            msg.time = currentDiffTime(msg.time);
            pthread_mutex_unlock(&m);
            f(&msg, fp);
        }
    }
    unsigned currentTime()
    {
#ifdef _WIN32
        return timeGetTime();
#else
        struct timespec ts = { 0, 0 };
        clock_gettime(CLOCK_MONOTONIC, &ts);
        time_t cur = ts.tv_sec * 1000000000LL + ts.tv_nsec;
        return (unsigned)(cur / 1000000LL);
#endif
    }

    unsigned currentDiffTime(unsigned t)
    {
        unsigned cur = currentTime();
        if (cur < t)
            return cur + (UINT_MAX - t);
        return cur - t;
    }
};

#endif //MESSAGEQUEUE_H