#ifndef WEBSERVER_TIMERNODE_H
#define WEBSERVER_TIMERNODE_H

#include "base/Mutex.h"
#include "http/HttpData.h"

#include <memory>
#include <functional>

class HttpData;

// 拥有HttpData线程的定时器
class TimerNode 
{
public:
    static int currentTime_t; // 当前时间

    TimerNode(std::shared_ptr<HttpData> httpData,int timeout);
    ~TimerNode();

    bool isDeleted() const { return deleted_t; }

    size_t getExpireTime() { return expiredTime_t; }

    bool isExpired() const { return expiredTime_t < currentTime_t; }

    void deleted();

    std::shared_ptr<HttpData> getHttpData() { return httpData_t; }

    static void currentTime();
 
private:
    bool deleted_t;
    size_t expiredTime_t;
    std::shared_ptr<HttpData> httpData_t;
    
};

struct TimerCmp {
    bool operator()(std::shared_ptr<TimerNode> &first, std::shared_ptr<TimerNode> &second) const {
        return first->getExpireTime() > second->getExpireTime();
    }
};


#endif //WEBSERVER_BASE_DATRTIME_TIMERNODE_H