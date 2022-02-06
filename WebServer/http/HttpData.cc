#include "HttpData.h"

void HttpData::closeTimer()
{
    // 判断Timer是否还在， 有可能已经超时释放
    if (timer_t.lock())
    {
        std::shared_ptr<TimerNode> tempTimer(timer_t.lock());
        tempTimer->deleted();

        // 断开weak_ptr
        timer_t.reset();
    }
}

void HttpData::setTimer(std::shared_ptr<TimerNode> timer)
{
    timer_t = timer;
}
