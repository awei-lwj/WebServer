#ifndef WEBSERVER_BASE_LOGGING_ASYNCLOGGING_H
#define WEBSERVER_BASE_LOGGING_ASYNCLOGGING_H

#include "../CountDownLatch.h"
#include "../Mutex.h"
#include "../thread/Thread.h"
#include "../Noncopyable.h"

#include "LogStream.h"

#include <functional>
#include <string>
#include <vector>

class AsyncLogging : public noncopyable
{
public:
    AsyncLogging(const std::string baseName, int flushInterval = 2);
    ~AsyncLogging()
    {
        if (running)
            stop();
    }

    void append(const char *logline, int len);

    void start()
    {
        running = true;
        thread_t.start();
        latch_t.wait();
    }

    void stop()
    {
        running = false;
        condition_t.notify();
        thread_t.join();
    }

private:
    /**
     * @brief
     * 1. 每个进程最好只写一个日志文件
     * 2. 尽可能避免磁盘I/O这对one loop per thread模型的非阻塞服务端程序尤为重要
     * 3. 接下来使用double buffering，前端负责往buffer A填数据，后端负责将buffer B写入文件，
     *    满了之后将其交换
     * 4. 实际实现用了四个缓冲区，这样可以进一步减少或避免日志前端的等待
     * 5. 使用shared_ptr进行对象生命期的自动管理
     */

    void threadFunction();

    // FixedBuffer所实现的一个日志缓冲区，大小为4MB
    typedef FixedBuffer<biggerBuffer> Buffer;

    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;

    BufferPtr currentBuffer; // 现在所使用的buffer
    BufferPtr prepareBuffer; // 预备buffer
    BufferVector buffers;    // 已经填满的buffer序列

    const int flushInterval_t;
    bool running;
    std::string baseName;

    Thread thread_t;
    MutexLock mutex_t;
    Condition condition_t;
    CountDownLatch latch_t;
};



#endif // WEBSERVER_BASE_LOGGING_ASYNCLOGGING_H
