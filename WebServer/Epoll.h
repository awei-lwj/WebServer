#ifndef WEBSERVER_EPOLL_H
#define WEBSERVER_EPOLL_H

#include "Socket.h"
#include "TimerQueue.h"

#include "http/HttpData.h"

#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <memory>

class Epoll
{
public:
    static int init(int maxEvents);

    static int epollAdd(int epoll_fd, int fd, int events, std::shared_ptr<HttpData> httpData);

    static int epollModify(int epoll_fd, int fd, int events, std::shared_ptr<HttpData> httpData);

    static int epollDeleted(int epoll_fd, int fd, int events);

    static void handleConnection(const ServerSocket &serverSocket);

    static std::vector<std::shared_ptr<HttpData>> poll(const ServerSocket &serverSocket, int maxEvents, int timeout);
    
public:
    // 用unordered_map以及定义一个线程最大可以处理的事件数目保证了one peer through loop
    // 相当于sharedChannel
    static std::unordered_map<int, std::shared_ptr<HttpData>> httpDataMap;

    static const int MAX_EVENTS;
    static epoll_event *events;

    // 定时器实现事件最小堆的管理
    static TimerQueue timerQueue;

    // 用unordered_map以及定义一个线程最大可以处理的事件数目保证了one peer through loop
    const static int DEFAULT_EVENTS;
};

#endif // WEBSERVER_EPOLL_H