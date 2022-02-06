#ifndef WEBSERVER_REACTOR_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_REACTOR_EVENTLOOPTHREADPOOL_H

#include "EventLoopThread.h"
#include "../base/Noncopyable.h"

#include <memory>
#include <vector>

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop *baseLoop, int numThreads);

    ~EventLoopThreadPool() { LOG << "~EventLoopThreadPool()"; }
    void start();

    EventLoop *getNextLoop();

private:
    EventLoop *baseLoop_t;
    bool started_t;
    int numThreads_t;
    int next_t;
    std::vector<std::shared_ptr<EventLoopThread>> threads_t;
    std::vector<EventLoop *> loops_t;
};

#endif // WEBSERVER_REACTOR_EVENTLOOPTHREADPOOL_H