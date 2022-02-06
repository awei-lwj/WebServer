#ifndef WEBSERVER_BASE_CONDITION_H
#define WEBSERVER_BASE_CONDITION_H

#include "Mutex.h"
#include "Noncopyable.h"

#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <cstdint>

class Condition : public noncopyable
{
public:
    explicit Condition(MutexLock &mutex_) : mutex(mutex_)
    {
        pthread_cond_init(&pcond, NULL);
    }

    ~Condition()
    {
        pthread_cond_destroy(&pcond);
    }

    void wait()
    {
        pthread_cond_wait(&pcond, mutex.get());
    }

    // 檢測是否超時間
    bool waitForSeconds(int seconds)
    {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec = seconds;

        return ETIMEDOUT == pthread_cond_timedwait(&pcond, mutex.get(), &abstime);
    }

    void notify()
    {
        pthread_cond_signal(&pcond);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&pcond);
    }

private:
    MutexLock &mutex;
    pthread_cond_t pcond;
};

#endif // WEBSERVER_BASE_CONDITION_H