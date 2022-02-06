#ifndef WEBSERVER_BASE_LOGGING_FILEUTIL_H
#define WEBSERVER_BASE_LOGGING_FILEUTIL_H

#include "../Noncopyable.h"
#include <string>

class AppendFile : public noncopyable
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();

    //直接向文件写
    void append(const char *logline, const size_t len);

    // 进行文件刷新
    void flush();

private:
    size_t write(const char *logline, size_t len);
    FILE *fp_t;
    char buffer_t[10 * 1024];
};

#endif // define WEBSERVER_BASE_LOGGING_FILEUTIL_H