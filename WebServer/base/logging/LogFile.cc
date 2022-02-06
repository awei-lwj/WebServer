#include "LogFile.h"
#include "FileUtil.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

LogFile::LogFile(const std::string &basename_t, int flushEvery_t)
    : basename(basename_t),
      flushEvery(flushEvery_t),
      count(0),
      mutex_t(new MutexLock)
{
    file_t.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::append(const char *logline, int len)
{
    MutexLockGuard lock(*mutex_t);
    append_unlock(logline, len);
}

void LogFile::flush()
{
    MutexLockGuard lock(*mutex_t);
    file_t->flush();
}

void LogFile::append_unlock(const char *logline, int len)
{
    file_t->append(logline, len);
    ++count;
    if (count >= flushEvery)
    {
        count = 0;
        file_t->flush();
    }
}
