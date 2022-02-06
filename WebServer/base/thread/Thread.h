#ifndef WEBSERVER_BASE_THREAD_THREAD_H
#define WEBSERVER_BASE_THREAD_THREAD_H

// 实现服务器的线程,shared_ptr控制对象的生命期
// 用于日志的输出
#include "../Noncopyable.h"
#include "../CountDownLatch.h"
#include "Atomic.h"

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>

class Thread : public noncopyable
{
public:
    typedef std::function<void()> ThreadFunction;
    explicit Thread(ThreadFunction, const std::string &name = std::string());
    ~Thread();

    void start();
    int join();

    bool started() const
    {
        return started_t;
    }

    pid_t tid() const
    {
        return tid_t;
    }

    const std::string &name() const
    {
        return name_t;
    }

    static int numCreated() { return numCreated_t.get(); }

private:
    void setDefaultName();

    bool started_t;
    bool joined;

    pthread_t pthreadID;
    pid_t tid_t;

    ThreadFunction fun;
    std::string name_t;
    static AtomicInt64 numCreated_t;

    CountDownLatch latch;
};

#endif // WEBSERVER_BASE_THREAD_THREAD_H