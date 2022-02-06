#include "EventLoopThread.h"
#include <functional>

EventLoopThread::EventLoopThread()
    : looping_t(NULL),
      exiting(false),
      thread_t(bind(&EventLoopThread::threadFunction, this), "EventLoopThread"),
      mutex_t(),
      condition_t(mutex_t) {}

EventLoopThread::~EventLoopThread()
{
    exiting = true;
    if (looping_t != NULL)
    {
        looping_t->quit();
        thread_t.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    assert(!thread_t.started());
    thread_t.start();
    {
        MutexLockGuard lock(mutex_t);
        // 一直等到threadFun在Thread里真正跑起来
        while (looping_t == NULL)
            condition_t.wait();
    }
    return looping_t;
}

void EventLoopThread::threadFunction()
{
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_t);
        looping_t = &loop;
        condition_t.notify();
    }

    loop.loop();
    // assert(exiting_);
    looping_t = NULL;
}