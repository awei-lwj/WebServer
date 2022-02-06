#include "CurrentThread.h"
#include "Thread.h"

#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <sys/syscall.h>

#include <iostream>
// 实现服务器线程,weak_ptr不控制对象的生命期，但是它知道对象是否还活着

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char *t_threadName = "default";
};

namespace threadResource
{
    pid_t gettid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void afterFork()
    {
        CurrentThread::t_cachedTid = 0;
        CurrentThread::t_threadName = "main";
        CurrentThread::tid();
    }

    class ThreadNameInitializer
    {
    public:
        ThreadNameInitializer()
        {
            CurrentThread::t_threadName = "main";
            CurrentThread::tid();
            pthread_atfork(NULL, NULL, &afterFork);
        }
    };

    ThreadNameInitializer init;

    struct ThreadData
    {
        typedef Thread::ThreadFunction ThreadFun;
        ThreadFun func;
        std::string name;
        pid_t *tid;
        CountDownLatch *latch;

        ThreadData(ThreadFun func_t,
                   const std::string &name_t,
                   pid_t *tid_t,
                   CountDownLatch *latch_t)
            : func(func_t), name(name_t), tid(tid_t), latch(latch_t)
        {
        }

        void runInThread()
        {
            *tid = CurrentThread::tid();
            tid = NULL;
            latch->countDown();
            latch = NULL;

            CurrentThread::t_threadName = name.empty() ? "AWEI" : name.c_str();
            ::prctl(PR_SET_NAME, CurrentThread::t_threadName);

            // TODO: Exception handling?
            func();
            CurrentThread::t_threadName = "finished";
        }

    }; // struct thread_data

    void *startThread(void *obj)
    {
        ThreadData *data = static_cast<ThreadData *>(obj);
        data->runInThread();
        delete data;
        return NULL;
    }

}; // namespace threadResource

void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = threadResource::gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d", t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
    return tid() == ::getpid();
}

AtomicInt64 Thread::numCreated_t;

Thread::Thread(ThreadFunction func_t, const std::string &n)
    : started_t(false),
      joined(false),
      pthreadID(0),
      tid_t(0),
      fun(std::move(func_t)),
      name_t(n),
      latch(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_t && !joined)
    {
        pthread_detach(pthreadID);
    }
}

void Thread::setDefaultName()
{
    int num = numCreated_t.incrementAndGet();
    if (name_t.empty())
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_t = buf;
    }
}

void Thread::start()
{
    assert(!started_t);
    started_t = true;

    threadResource::ThreadData *data = new threadResource::ThreadData(fun, name_t, &tid_t, &latch);
    if (pthread_create(&pthreadID, NULL, &threadResource::startThread, data))
    {
        started_t = false;
        delete data;
    }
    else
    {
        latch.wait();
        assert(tid_t > 0);
    }
}

int Thread::join()
{
    assert(started_t);
    assert(!joined);
    joined = true;
    return pthread_join(pthreadID, NULL);
}
