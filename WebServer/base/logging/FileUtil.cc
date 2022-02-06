#include "FileUtil.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

AppendFile::AppendFile(string filename) : fp_t(fopen(filename.c_str(), "ae"))
{
    setbuffer(fp_t, buffer_t, sizeof(buffer_t));
}

AppendFile::~AppendFile()
{
    fclose(fp_t);
}

void AppendFile::append(const char *logline, const size_t len)
{
    size_t n = this->write(logline, len);
    size_t remain = len - n;

    while (remain > 0)
    {
        size_t temp = this->write(logline + n, remain);
        if (temp == 0)
        {
            int error = ferror(fp_t);
            if (error)
            {
                fprintf(stderr, "AppendFile::append() failed !! Please check!!\n");
            }
            break;
        }

        n += temp;
        remain = len - n;
    }
}

void AppendFile::flush()
{
    fflush(fp_t);
}

size_t AppendFile::write(const char *logline, size_t len)
{
    return fwrite_unlocked(logline, 1, len, fp_t);
}