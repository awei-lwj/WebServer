#ifndef WEBSERVER_BASE_LOGGING_LOGFILE_H
#define WEBSERVER_BASE_LOGGING_LOGFILE_H

// 日志文件压缩与归档（archive）不是日志库应有的功能，而应该交给专门的脚本去做，这样C++和Java的服务程序
// 可以共享这一基础设施。

#include "../Mutex.h"
#include "../Noncopyable.h"
#include "FileUtil.h"

#include <memory>
#include <string>

class LogFile : public noncopyable
{
public:
    LogFile(const std::string &basename, int flushEvery = 1024);
    ~LogFile();

    void append(const char *logline, int len);
    void flush();
    bool rollFile();

private:
    void append_unlock(const char *logline, int len);

    const std::string basename;
    const int flushEvery;

    int count;
    std::unique_ptr<MutexLock> mutex_t;
    std::unique_ptr<AppendFile> file_t;
};

#endif // WEBSERVER_BASE_LOGGING_LOGFILE_H