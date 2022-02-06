#include "Epoll.h"
#include "../base/logging/Logging.h"

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <deque>
#include <queue>
#include <cstring>
#include <arpa/inet.h>

const int eventsNum = 8192;
const int waitTime = 100000;

Epoll::Epoll()
    : epollFd(epoll_create1(EPOLL_CLOEXEC)),
      events_t(eventsNum)
{
    assert(epollFd > 0);
}

Epoll::~Epoll()
{
}

// 注册新描述符
void Epoll::epollAdd(SharedChannel request, int timeout)
{
    int fd = request->fd();

    if (timeout > 0)
    {
        addTimer(request, timeout);
        fdHttpData[fd] = request->getOwner();
    }

    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->events();

    request->updateLastEvents();

    fdChannel[fd] = request;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        fdChannel[fd].reset();
    }
}

// 修改描述符状态
void Epoll::epollModify(SharedChannel request, int timeout)
{
    if (timeout > 0)
    {
        addTimer(request, timeout);
    }

    int fd = request->fd();

    if (!request->updateLastEvents())
    {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->events();
        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            perror("epoll_mod error");
            fdChannel[fd].reset();
        }
    }
}

// 从epoll中删除描述符
void Epoll::epollDelete(SharedChannel request)
{
    int fd = request->fd();

    struct epoll_event event;

    event.data.fd = fd;
    event.events = request->lastevents();

    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
    }

    fdChannel[fd].reset();
    fdHttpData[fd].reset();
}
// 分发处理函数
std::vector<SharedChannel> Epoll::getEventsRequest(int eventsNum)
{
    std::vector<SharedChannel> requestData;

    for (int i = 0; i < eventsNum; i++)
    {
        int fd = events_t[i].data.fd;

        SharedChannel curRequest = fdChannel[fd];

        if (curRequest)
        {
            curRequest->setRevents(events_t[i].events);
            curRequest->setEvents(0);
            requestData.push_back(curRequest);
        }
        else
        {
            LOG << "SharedChannel curRequest is in invalid";
        }
    }

    return requestData;
}

void Epoll::addTimer(SharedChannel request,int timeout)
{
    shared_ptr<HttpData> temp = request->getOwner();
    if(temp)
        timerQueue_t.addTimer(temp,timeout);
    else
        LOG << "timer add fail";
}

void Epoll::handleExpired()
{
    return timerQueue_t.handleExpiredEvents();
}