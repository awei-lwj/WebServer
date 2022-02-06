#include "ThreadPool.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <cstdio>

ThreadPool::ThreadPool(const std::string &name)
    : mutex_t(),
      cond_t(mutex_t),
      name_t(name),
      running(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads.empty());
    running = true;
    threads.reserve(numThreads);

    for (int i = 0; i < numThreads; i++)
    {
        char id[32];
        snprintf(id, sizeof(id), "%d", i);
        threads.push_back(new Thread(std::bind(&ThreadPool::runInThread, this), name_t + id));
        threads[i].start();
    }
}

void ThreadPool::stop()
{
    running = false;
    cond_t.notifyAll();
    for_each(threads.begin(), threads.end(), boost::bind(&Thread::join, _1));
}

void ThreadPool::run(const Task &task)
{
    if (threads.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_t); // 锁争用
        queue_t.push_back(task);
        cond_t.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(mutex_t);
    while (queue_t.empty() && running)
    {
        cond_t.wait();
    }

    Task task;

    if (!queue_t.empty())
    {
        task = queue_t.front();
        queue_t.pop_front();
    }

    return task;
}

void ThreadPool::runInThread()
{
    while(running)
    {
        Task task(take());
        if(task)
        {
            task();
        }
    }
}











