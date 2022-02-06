#include "EventThreadPool.h"
#include <iostream>
#include <pthread.h>
#include <sys/prctl.h>

EventThreadPool::EventThreadPool(int threadSize, int maxQueueSize)
    : condition_t(mutex_t),
      threadSize_t(threadSize),
      maxQueueSize_t(maxQueueSize),
      started(0),
      shutdown_t(0)
{
    if (threadSize <= 0 || threadSize > MAX_THREAD_SIZE)
    {
        threadSize_t = 8;
    }

    if (maxQueueSize <= 0 || maxQueueSize > MAX_QUEUE_SIZE)
    {
        maxQueueSize_t = MAX_QUEUE_SIZE;
    }

    threads.resize(threadSize_t);

    for (int i = 0; i < threadSize_t; i++)
    {
        if (pthread_create(&threads[i], NULL, worker, this) != 0)
        {
            LOG << "Failed to create thread";
            throw std::exception();
        }

        started++;
    }
}

EventThreadPool::~EventThreadPool()
{
}

bool EventThreadPool::append(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> func)
{
    if (shutdown_t)
    {
        LOG << "Thread has shutdown.";
    }

    MutexLockGuard guard(mutex_t);

    if (requestQueue.size() > maxQueueSize_t)
    {
        LOG << "maxQueueSize_t is" << maxQueueSize_t;
        LOG << "ThreadPool too much request";
        return false;
    }

    Events eventsTask;
    eventsTask.arg = arg;
    eventsTask.process = func;

    requestQueue.push_back(eventsTask);

    condition_t.notify();
    return true;
}

void EventThreadPool::shutdown(bool graceful)
{
    {
        MutexLockGuard guard(mutex_t);

        if (shutdown_t)
        {
            LOG << "EventThreadPool has been shutdown";
        }

        shutdown_t = graceful ? gracefulMode : immediateMode;
    }

    for (int i = 0; i < threadSize_t; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            LOG << "pthread_join error";
        }
    }
}

void *EventThreadPool::worker(void *args)
{
    EventThreadPool *pool = static_cast<EventThreadPool *>(args);

    // 退出线程
    if (pool == nullptr)
        return NULL;

    prctl(PR_SET_NAME, "EventLoopThread");

    pool->run(); // 执行线程主方法

    return NULL;
}

void EventThreadPool::run()
{
    while (true)
    {
        Events eventsTask;
        {
            MutexLockGuard guard(this->mutex_t);

            // 无任务 且未shutdown 则条件等待
            while (requestQueue.empty() && !shutdown_t)
            {
                condition_t.wait();
            }

            if ((shutdown_t == immediateMode) || (shutdown_t == gracefulMode && requestQueue.empty()))
            {
                break;
            }

            eventsTask = requestQueue.front(); // FIFO
            requestQueue.pop_front();
        }

        eventsTask.process(eventsTask.arg);
    }
}
