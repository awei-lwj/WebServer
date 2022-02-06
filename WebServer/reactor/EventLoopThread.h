#ifndef WEBSERVER_EVENETLOOPTHREAD_H
#define WEBSERVER_EVENETLOOPTHREAD_H
#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "base/thread/Thread.h"

#include "EventLoop.h"

class EventLoop;

class EventLoopThread : public noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunction();

    EventLoop *looping_t;
    bool exiting;
    Thread thread_t;
    MutexLock mutex_t;
    Condition condition_t;
};

#endif // WEBSERVER_EVENETLOOPTHREAD_H