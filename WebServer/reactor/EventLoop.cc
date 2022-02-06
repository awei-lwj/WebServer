#include "EventLoop.h"
#include "../base/logging/Logging.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>

__thread EventLoop *loopInThisThread_t = 0;

EventLoop::EventLoop()
    : looping(false),
      quit_t(false),
      eventCallback(false),
      threadID(CurrentThread::tid()),
      wakeupFd(createEventfd()),
      epoller_t(new Epoll()),
      wakeupChannel(new Channel(this, wakeupFd)),
      callPendingFunctions_t(false)
{
    LOG << "EventLoop created " << this << " in thread " << threadID;

    if (loopInThisThread_t)
    {
        LOG << "Another EventLoop " << loopInThisThread_t
            << "exists in this thread. " << threadID;
    }
    else
    {
        loopInThisThread_t = this;
    }

    wakeupChannel->setEvents(EPOLLIN | EPOLLET);
    wakeupChannel->setReadCallback(bind(&EventLoop::callbackRead, this));
    wakeupChannel->setConnectCallback(bind(&EventLoop::callbackConnect, this));
    epoller_t->epollAdd(wakeupChannel, 0);
}

EventLoop::~EventLoop()
{
    assert(!looping);
    loopInThisThread_t = NULL;
}

void EventLoop::loop() 
{
    assert(!looping);
    assert(isInLoopThread());
    looping = true;
    quit_t = false;

    LOG << "EventLoop " << this << " start looping";

    std::vector<SharedChannel> ret;
    while (!quit_t)
    {
        ret.clear();
        ret = epoller_t->poll();

        eventCallback = true;
        for (auto &it : ret)
            it->handleEvent();
        eventCallback = true;
        doPendingFunctors();
        epoller_t->handleExpired();
    }

    looping = false;
}

int EventLoop::createEventfd() 
{
    int ans = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    if (ans < 0)
    {
        LOG << "Failed to create eventfd";
        abort();
    }

    return ans;
}

void EventLoop::quit()
{
    quit_t = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor &&cb)
{
    if (isInLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor &&cb)
{
    {
        MutexLockGuard lock(mutex_t);
        pendingFunctors_t.emplace_back(std::move(cb));
    }

    if (!isInLoopThread())
        wakeup();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd, (char *)(&one), sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::callbackRead()
{
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    wakeupChannel->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::callbackConnect()
{
    updateEpoller(wakeupChannel,0);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callPendingFunctions_t = true;

    {
        MutexLockGuard lock(mutex_t);
        functors.swap(pendingFunctors_t);
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    callPendingFunctions_t = false;
}
