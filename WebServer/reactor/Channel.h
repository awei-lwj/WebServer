#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include "../http/HttpData.h"
#include "EventLoop.h"
#include "../TimerQueue.h"

#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class EventLoop;
class HttpData;

typedef std::shared_ptr<Channel> SharedChannel; 
  

// 每个Channel对象自始至终只属于一个EventLoop对象，
// 因此每个Channel对象都只属于一个IO线程，只负责一个文件描述符（fd）的IO事件分发， 
// 但他并不拥有这个fd，而且也不会关闭这个fd
class Channel
{
private:
    typedef std::function<void()> EventCallback;

    EventLoop *loop_t;
    int fd_t;

    int events_t;
    int revents_t;
    int lastevents_t;

    std::weak_ptr<HttpData> owner_t;

    EventCallback readCallback_t;
    EventCallback writeCallback_t;
    EventCallback errorCallback_t;
    EventCallback connectCallback_t;

public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    int fd() const { return fd_t; }
    void setFd(int fd) { fd_t = fd; }

    int events()     const { return events_t; }
    int revents()    const { return revents_t; }
    int lastevents() const { return lastevents_t; }

    void setEvents(int events)   { events_t = events; }
    void setRevents(int revents) { revents_t = revents; }

    bool updateLastEvents()
    {
        bool ans = (lastevents_t == events_t);
        lastevents_t = events_t;
        return ans;
    }

    void setOwner(std::shared_ptr<HttpData> owner)
    {
        owner_t = owner;
    }

    std::shared_ptr<HttpData> getOwner()
    {
        std::shared_ptr<HttpData> ret(owner_t);
        return ret;
    }


    void setReadCallback(const EventCallback &readCallback)
    {
        readCallback_t = readCallback;
    }

    void setWriteCallback(const EventCallback &writeCallback)
    {
        writeCallback_t = writeCallback;
    }

    void setErrorCallback(const EventCallback &errorCallback)
    {
        errorCallback_t = errorCallback;
    }

    void setConnectCallback(const EventCallback &connectCallback)
    {
        connectCallback_t = connectCallback;
    }

    // 负责将不同的IO事件进行回调
    void handleEvent();
};


#endif // WEBSERVER_CHANNEL_H

