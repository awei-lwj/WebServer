#include "Channel.h"
#include "EventLoop.h"
#include "../base/logging/Logging.h"

#include <sys/epoll.h>
#include <sstream>

Channel::Channel(EventLoop *loop, int fd)
    : loop_t(loop),
      fd_t(fd),
      events_t(0),
      lastevents_t(0)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent()
{
    events_t = 0;

    if((revents_t & EPOLLHUP) && !(revents_t & EPOLLIN))
    {
        LOG << "Channel::handleEvent() EPOLLHUP && !EPOLLIN";
        events_t = 0;
        return ;   
    }

    if ( revents_t & EPOLLERR)
    {
        if(errorCallback_t) errorCallback_t();
        events_t = 0;
        return ;
    }

    if(revents_t & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if(readCallback_t) readCallback_t();
    }

    if(revents_t & EPOLLOUT)
    {
        if(writeCallback_t) writeCallback_t();
    }

    if(connectCallback_t) connectCallback_t();

    return ;
}
