// 最后的logging封装
#ifndef WEBSERVER_BASE_LOGGING_LOGGING_H
#define WEBSERVER_BASE_LOGGING_LOGGING_H

#include "LogStream.h"

#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <string>

class AsyncLogging;

class Logger
{
public:
    Logger(const char *filename, int line);
    ~Logger();

    LogStream &stream() { return impl.stream_t; }

    static void setlogFileName(std::string filename) { logFilename = filename; }
    static std::string getLogFileName() { return logFilename; }

private:
    class Implicit
    {
    public:
        Implicit(const char *filename, int line);
        void formatTime();

        LogStream stream_t;
        int line_t;
        std::string baseName;
    };

    Implicit impl;
    static std::string logFilename;
};

#define LOG Logger(__FILE__, __LINE__).stream()

#endif // WEBSERVER_BASE_LOGGING_LOGGING_H