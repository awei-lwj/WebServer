#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H

#include "../http/HttpData.h"
#include "Channel.h"
#include "../TimerQueue.h"


#include <sys/epoll.h>
#include <memory.h>
#include <unordered_map>
#include <vector>


// IO multiplexing
class Channel;
typedef std::shared_ptr<Channel> SharedChannel;

class Epoll
{
public:
    Epoll();
    ~Epoll();

    void epollAdd(SharedChannel request, int timeout);
    void epollModify(SharedChannel request, int timeout);
    void epollDelete(SharedChannel request);

    std::vector<std::shared_ptr<Channel>> poll();
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int eventsNum);

    int getEpollFd() { return epollFd; }

    void addTimer(std::shared_ptr<Channel> request_data, int timeout);

    void handleExpired();

private:
    static const int maxFd = 100000;
    int epollFd;

    std::vector<epoll_event> events_t;
    std::shared_ptr<Channel> fdChannel[maxFd];
    std::shared_ptr<HttpData> fdHttpData[maxFd];

    TimerQueue timerQueue_t;
};

#endif // WEBSERVER_EPOLL_H