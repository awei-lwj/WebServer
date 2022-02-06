#ifndef WEBSERVER_BASE_LOGGING_LOGSTREAM_H
#define WEBSERVER_BASE_LOGGING_LOGSTREAM_H

#include "../Noncopyable.h"
#include <assert.h>
#include <cstring>
#include <string>

class AsyncLogging;
const int smallerBuffer = 4096;
const int biggerBuffer = 4096 * 1024;

template <int SIZE>
class FixedBuffer : public noncopyable
{
public:
    FixedBuffer() : cur(data_t) {}

    ~FixedBuffer() {}

    void append(const char *buf, size_t len)
    {
        if (avail() > static_cast<int>(len))
        {
            memcpy(cur, buf, len);
            cur += len;
        }
    }

    const char *data() const { return data_t; }
    int length() const { return static_cast<int>(cur - data_t); }
    char *current() const { return cur; }
    int avail() const { return static_cast<int>(end() - cur); }

    void add(size_t len) { cur += len; }

    void reset() { cur = data_t; }
    void bzero() { memset(data_t, 0, sizeof(data_t)); }

private:
    const char *end() const
    {
        return data_t + sizeof(data_t);
    }

    char data_t[SIZE];
    char *cur;
};

class LogStream : public noncopyable
{
public:
    typedef FixedBuffer<smallerBuffer> Buffer;

    // 现在开始重载<<输出符号
    // 为了与标准库IO操作一致，重载 << 操作符函数应把ostream&作为其第一个参数，
    // 对类类型const对象的引用作为第二个参数，并返回对ostream形参的引用。
    LogStream &operator<<(bool v)
    {
        buffer_t.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream &operator<<(short);
    LogStream &operator<<(unsigned short);
    LogStream &operator<<(int);
    LogStream &operator<<(unsigned int);

    LogStream &operator<<(long);
    LogStream &operator<<(unsigned long);
    LogStream &operator<<(long long);
    LogStream &operator<<(unsigned long long);

    LogStream &operator<<(const void *);

    LogStream &operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }

    LogStream &operator<<(double);
    LogStream &operator<<(long double);

    LogStream &operator<<(char v)
    {
        buffer_t.append(&v, 1);
        return *this;
    }

    LogStream &operator<<(const char *str)
    {
        if (str)
            buffer_t.append(str, strlen(str));
        else
            buffer_t.append("(null)", 6);
        return *this;
    }

    LogStream &operator<<(const unsigned char *str)
    {
        return operator<<(reinterpret_cast<const char *>(str));
    }

    LogStream &operator<<(const std::string &v)
    {
        buffer_t.append(v.c_str(), v.size());
        return *this;
    }

    void append(const char* data, int len) { buffer_t.append(data, len); }
    const Buffer& buffer() const { return buffer_t; }
    void resetBuffer() { buffer_t.reset(); }

private:
    void staticCheck();

    template <typename T>
    void formatInteger(T);

    Buffer buffer_t;

    static const int maxNumbericSize = 64;
};

#endif // WEBSERVER_BASE_LOGGING_LOGSTREAM_H