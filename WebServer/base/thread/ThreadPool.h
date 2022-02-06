#ifndef WEBSERVER_BASE_THREAD_THREADPOOL_H
#define WEBSERVER_BASE_THREAD_THREADPOOL_H

#include "../Condition.h"
#include "../Mutex.h"
#include "../Noncopyable.h"
#include "Thread.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <functional>
#include <deque>

// 计算线程的线程池,以及服务器日志的线程
class ThreadPool : public noncopyable
{
public:
    typedef std::function<void()> Task;

    explicit ThreadPool(const std::string &name = std::string());
    ~ThreadPool();

    void start(int numThreads);
    void stop();
    void run(const Task &task);

private:
    void runInThread();
    Task take();

    MutexLock mutex_t;
    Condition cond_t;

    std::string name_t;
    boost::ptr_vector<Thread> threads;
    std::deque<Task> queue_t;
    bool running;
};

#endif // WEBSERVER_BASE_THREAD_THREADPOOL_H