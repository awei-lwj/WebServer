#ifndef WEBSERVER_REACTOR_EVENTLOOP_H
#define WEBSERVER_REACTOR_EVENTLOOP_H

#include "Channel.h"
#include "Epoll.h"
#include "../TimerQueue.h"
#include "../Util.h"

#include "../base/thread/Thread.h"
#include "../base/thread/CurrentThread.h"
#include "../base/logging/Logging.h"
#include "../Socket.h"

#include <functional>
#include <memory>
#include <vector>
#include <iostream>
#include <assert.h>

using namespace std;

class Channel;
typedef std::shared_ptr<Channel> SharedChannel;

// Event Loop是一个程序结构，用于等待和发送消息和事件
// 负责主线程与其他进程（主要是各种I/O操作）的通信
class EventLoop
{
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();

    int createEventfd();

    void loop();
    void quit();

    void runInLoop(Functor &&cb);
    void queueInLoop(Functor &&cb);

    bool isInLoopThread() const
    {
        return threadID == CurrentThread::tid();
    }

    void assertInLoopThread() const
    {
        assert(isInLoopThread());
    }

    void removeFromEpoller(shared_ptr<Channel> channel)
    {
        epoller_t->epollDelete(channel);
    }

    void shutdown(shared_ptr<Channel> channel)
    {
    }

    void updateEpoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        epoller_t->epollModify(channel, timeout);
    }

    void addToEpoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        epoller_t->epollAdd(channel, timeout);
    }

private:
    bool looping;
    bool quit_t;
    bool eventCallback;

    const pid_t threadID;
    int wakeupFd;
    MutexLock mutex_t;

    shared_ptr<Epoll> epoller_t;
    shared_ptr<Channel> wakeupChannel;

    vector<Functor> pendingFunctors_t;
    bool callPendingFunctions_t;

    void wakeup();
    void callbackRead();
    void callbackConnect();
    void doPendingFunctors();
};

#endif // WEBSERVER_REACTOR_EVENTLOOP_H