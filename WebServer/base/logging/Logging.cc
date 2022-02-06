#include "Logging.h"
#include "AsyncLogging.h"
#include "../thread/CurrentThread.h"
#include "../thread/Thread.h"

#include <assert.h>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <ctime>

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger;

std::string Logger::logFilename = "./AweiWebServer.log";

void once_init(void)
{
    AsyncLogger = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger->start(); 
}

Logger::Implicit::Implicit(const char *fileName, int line)
  : stream_t(),
    line_t(line),
    baseName(fileName)
{
    formatTime();
}

void output(const char* message,int len)
{
    pthread_once(&once_control,once_init);
    AsyncLogger->append(message,len);
}

void Logger::Implicit::formatTime()
{
    struct timeval tv;
    time_t time = 0;

    char str_t[26] = {0};
    gettimeofday(&tv,NULL);

    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);

    strftime(str_t,26, "%Y-%m-%d %H:%M:%S\n",p_time);
    stream_t << str_t;
}

Logger::Logger(const char* filename,int line) : impl(filename,line)
{ }

Logger::~Logger()
{
    impl.stream_t << "  ---  "<<impl.baseName<<": "<<impl.line_t<<'\n';
    const LogStream::Buffer& buf(stream().buffer());
    output(buf.data(),buf.length());
}

