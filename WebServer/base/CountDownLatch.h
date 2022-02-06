#ifndef WEBSERVER_COUNTDOWNLATCH_H
#define WEBSERVER_COUNTDOWNLATCH_H

/**
 * @brief
 * 保证MutexLock和MutexLockGuard先于Condition中先构造出来，并作为传递参数给Condition
 * 并且实现计数型引用
 */

#include "Mutex.h"
#include "Condition.h"
#include "Noncopyable.h"

class CountDownLatch : public noncopyable
{
public:
    explicit CountDownLatch(int count) : mutex(),
                                         condition(mutex),
                                         count(count)
    {
    }

    void wait()
    {
        MutexLockGuard lock(mutex);
        while (count > 0)
        {
            condition.wait();
        }
    }

    void countDown()
    {
        MutexLockGuard lock(mutex);
        --count;
        if (count == 0)
        {
            condition.notifyAll();
        }
    }

    int getCount() const
    {
        MutexLockGuard lock(mutex);
        return count;
    }

private:
    mutable MutexLock mutex;
    Condition condition;
    int count;
};

#endif // WEBSERVER_COUNTDOWNLATCH_H