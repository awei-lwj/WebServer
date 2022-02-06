#ifndef WEBSERVER_HTTPDATATHREADPOOL_H
#define WEBSERVER_HTTPDATATHREADPOOL_H

#include "base/Noncopyable.h"
#include "base/Mutex.h"
#include "base/Condition.h"
#include "base/logging/Logging.h"


#include <vector>
#include <list>
#include <functional>
#include <pthread.h>
#include <memory>

const int MAX_THREAD_SIZE = 1024;
const int MAX_QUEUE_SIZE = 10000;

typedef enum{
    immediateMode = 1,
    gracefulMode  = -1,
}ShutDownMode;

// EventLoop+EventLoopThread的简易版本
// 这里的event为httpData
struct Events 
{
    std::function<void(std::shared_ptr<void>)> process;     // Server::do_request;
    std::shared_ptr<void> arg;                              // HttpData对象
};

// 该线程池用来处理httpData的线程池，与上面的服务器日志线程略微有点不同
// EventLoop+EventLoopThread+EventLoopThreadPool的简易版本
class EventThreadPool : public noncopyable
{
public:
    EventThreadPool(int threadSize,int maxQueueSize);

    ~EventThreadPool();

    bool append(std::shared_ptr<void> arg,std::function<void(std::shared_ptr<void>)> func);

    void shutdown(bool graceful);

private:

    // 线程池中的线程的互斥
    MutexLock mutex_t;
    Condition condition_t;

    // 线程池属性
    int threadSize_t;
    int maxQueueSize_t;
    int started;
    int shutdown_t;

    // 线程以及请求队列
    std::vector<pthread_t> threads;
    std::list<Events> requestQueue;

    static void *worker(void *args);

    void run();

};

#endif // WEBSERVER_HTTPDATATHREADPOOL_H