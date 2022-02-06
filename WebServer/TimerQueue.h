#ifndef WEBSERVER_TIMER_QUEUE_H
#define WEBSERVER_TIMER_QUEUE_H

#include "base/Mutex.h"
#include "base/Copyable.h"
#include "http/HttpData.h"

#include "TimerNode.h"

#include <queue>
#include <deque>
#include <memory>


class TimerNode;


// 使用 priority queue 实现的最小堆结构管理定时器，使用标记删除，以支持惰性删除，提高性能
class TimerQueue : public copyable
{
public:
    typedef std::shared_ptr<TimerNode> SharedTimerNode;

    void addTimer(std::shared_ptr<HttpData> httpData, int timeout);

    void handleExpiredEvents();

    const static int DEFAULT_TIMEOUT;

private:
    std::priority_queue<SharedTimerNode, std::deque<SharedTimerNode>, TimerCmp> timerQueue;
    MutexLock mutex_t;
};

#endif // WEBSERVER_TIMER_QUEUE_H