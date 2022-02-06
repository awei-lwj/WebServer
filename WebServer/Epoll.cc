#include "Epoll.h"
#include "Util.h"

#include "base/logging/Logging.h"

#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include <cstdio>

// 相当于sharedChannel=
std::unordered_map<int, std::shared_ptr<HttpData>> Epoll::httpDataMap;

const int Epoll::MAX_EVENTS = 1000;

epoll_event *Epoll::events;

// 可读 | ET模 | 保证一个socket连接在任一时刻只被一个线程处理
const int Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);

TimerQueue Epoll::timerQueue;

int Epoll::init(int maxEvents)
{
    // Global functions ::epoll_create
    int epoll_fd = ::epoll_create(maxEvents);

    if (epoll_fd == -1)
    {
        LOG << "epoll create error";
        exit(-1);
    }
    events = new epoll_event[maxEvents];

    return epoll_fd;
}

int Epoll::epollAdd(int epollFd, int fd, int events, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = (EPOLLIN | EPOLLET);
    event.data.fd = fd;

    // 增加httpDataMap
    httpDataMap[fd] = httpData;
    int ret = ::epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);

    if (ret < 0)
    {
        LOG << "epoll add error";
        // 释放httpData
        httpDataMap[fd].reset();
        return -1;
    }

    return 0;
}

int Epoll::epollModify(int epoll_fd, int fd, int events, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    // 每次更改的时候也更新 httpDataMap
    httpDataMap[fd] = httpData;

    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    if (ret < 0)
    {
        LOG << "epoll mod error";

        httpDataMap[fd].reset(); // 释放httpData

        return -1;
    }

    return 0;
}

int Epoll::epollDeleted(int epoll_fd, int fd, int events)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;

    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);

    if (ret < 0)
    {
        LOG << "epoll del error";
        return -1;
    }
    auto it = httpDataMap.find(fd);

    if (it != httpDataMap.end())
    {
        httpDataMap.erase(it);
    }

    return 0;
}

void Epoll::handleConnection(const ServerSocket &serverSocket)
{
    std::shared_ptr<ClientSocket> tempClient(new ClientSocket);

    // epoll 是ET模式，循环接收连接
    // 需要将listen_fd设置为non-blocking
    // Event loop()
    while (serverSocket.accept(*tempClient) > 0)
    {
        // 设置非阻塞
        int ret = setnonBlocking(tempClient->fd);

        if (ret < 0)
        {
            std::cout << "setnonblocking error" << std::endl;
            tempClient->close();
            continue;
        }

        std::shared_ptr<HttpData> sharedHttpData(new HttpData);
        sharedHttpData->request_ = std::shared_ptr<HttpRequest>(new HttpRequest());
        sharedHttpData->response_ = std::shared_ptr<HttpResponse>(new HttpResponse());

        std::shared_ptr<ClientSocket> sharedClientSocket(new ClientSocket());
        sharedClientSocket.swap(tempClient);
        sharedHttpData->clientSocket_ = sharedClientSocket;
        sharedHttpData->epoll_fd = serverSocket.epoll_fd;

        epollModify(serverSocket.epoll_fd, sharedClientSocket->fd, DEFAULT_EVENTS, sharedHttpData);

        timerQueue.addTimer(sharedHttpData, TimerQueue::DEFAULT_TIMEOUT);
    }
}

std::vector<std::shared_ptr<HttpData>> Epoll::poll(const ServerSocket &serverSocket, int max_event, int timeout)
{
    int event_num = epoll_wait(serverSocket.epoll_fd, events, max_event, timeout);
    
    if (event_num < 0)
    {
        std::cout << "epoll_num=" << event_num << std::endl;
        std::cout << "epoll_wait error" << std::endl;
        std::cout << errno << std::endl;

        exit(-1);
    }

    std::vector<std::shared_ptr<HttpData>> httpDataTemp;

    // 遍历events集合,寻找活得事件
    for (int i = 0; i < event_num; i++)
    {
        int fd = events[i].data.fd;

        // 监听描述符
        if (fd == serverSocket.listen_fd)
        {
            handleConnection(serverSocket);
        }
        else
        {
            // 出错的描述符，移除定时器， 关闭文件描述符
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLHUP))
            {
                auto it = httpDataMap.find(fd);
                if (it != httpDataMap.end())
                {
                    // 将HttpData节点和TimerNode的关联分开，这样HttpData会立即析构，在析构函数内关闭文件描述符等资源
                    it->second->closeTimer();
                    httpDataMap.erase(it);
                }
                continue;
            }

            auto it = httpDataMap.find(fd);
            if (it != httpDataMap.end())
            {
                if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
                {
                    httpDataTemp.push_back(it->second);
                    it->second->closeTimer();
                    httpDataMap.erase(it);
                }
            }
            else
            {
                LOG << "长连接第二次连接未找到";
                ::close(fd);
                continue;
            }
            
        } // end if

    } // end while
    
    return httpDataTemp;
}
