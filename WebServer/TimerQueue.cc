#include "TimerQueue.h"
#include "TimerNode.h"
#include "base/logging/Logging.h"

//#include "Epoll.h"

const int TimerQueue::DEFAULT_TIMEOUT = 10 * 1000; // 10s

void TimerQueue::addTimer(std::shared_ptr<HttpData> httpData, int timeout)
{
    SharedTimerNode timerNode(new TimerNode(httpData, timeout));
    {
        // 将TimerNode和HttpData关联起来
        MutexLockGuard guard(mutex_t);
        timerQueue.push(timerNode);
        httpData->setTimer(timerNode);
    }
}

void TimerQueue::handleExpiredEvents()
{
    MutexLockGuard guard(mutex_t);

    // 更新当前时间
    // TimerNode::currentTime();

    LOG << "Start processing the timeout event";
    while (!timerQueue.empty())
    {
        SharedTimerNode temp = timerQueue.top();

        if (temp->isDeleted())
        {
            LOG << "the timeout event is deleted";
            timerQueue.pop();
        }
        else if (temp->isExpired())
        {
            LOG << "the timeout event is expired";
            timerQueue.pop();
        }
        else
        {
            break;
        }
    }
}