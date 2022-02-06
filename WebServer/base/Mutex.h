#ifndef WEBSERVER_BASE_MUTEX_H
#define WEBSERVER_BASE_MUTEX_H

// cstdio:不要用cout，因為cout是線程不安全的
#include "Noncopyable.h"
#include "cstdio"
#include <pthread.h>

// MutexLock用于封装critical section，用RAII手法封装互斥器的创建与销毁。
class MutexLock : public noncopyable
{
public:
    MutexLock() { pthread_mutex_init(&mutex, NULL); }
    ~MutexLock()
    {
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }

    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }
    pthread_mutex_t *get() { return &mutex; }

private:
    pthread_mutex_t mutex; // Mutex鎖

private:
    friend class Condition; // 友元类不受访问权限影响
};

// MutexLockGuard封装临界区的进入和退出
class MutexLockGuard : public noncopyable
{
public:
    explicit MutexLockGuard(MutexLock &mutex_) : mutex(mutex_) { mutex.lock(); }
    ~MutexLockGuard() { mutex.unlock(); };

private:
    MutexLock &mutex;
};

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A temporay object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif // WEBSERVER_BASE_MUTEX_H