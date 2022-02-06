#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_t(baseLoop), started_t(false),
      numThreads_t(numThreads),
      next_t(0)
{
    if (numThreads_t <= 0)
    {
        LOG << "numThreads_ <= 0";
        abort();
    }
}

void EventLoopThreadPool::start()
{
    baseLoop_t->assertInLoopThread();
    started_t = true;
    for (int i = 0; i < numThreads_t; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_t.push_back(t);
        loops_t.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseLoop_t->assertInLoopThread();
    assert(started_t);
    EventLoop *loop = baseLoop_t;
    if (!loops_t.empty())
    {
        loop = loops_t[next_t];
        next_t = (next_t + 1) % numThreads_t;
    }
    return loop;
}