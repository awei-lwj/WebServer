#include "TimerNode.h"
#include "base/logging/Logging.h"

#include <sys/time.h>
#include <unistd.h>

int TimerNode::currentTime_t = 0; // 当前时间

TimerNode::TimerNode(std::shared_ptr<HttpData> httpData,int timeout)
    : deleted_t(false),
      httpData_t(httpData)
{
    currentTime();
    expiredTime_t = currentTime_t + timeout;
}

TimerNode::~TimerNode()
{
}

void inline TimerNode::currentTime()
{
    struct timeval cur;
    gettimeofday(&cur, NULL);
    currentTime_t = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
}

void TimerNode::deleted()
{
    LOG << "Deleted TimerNode";
    httpData_t.reset();
    deleted_t = true;
}