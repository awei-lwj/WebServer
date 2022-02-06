#ifndef WEBSERVER_BASE_NONCOPYABLE_H
#define WEBSERVER_BASE_NONCOPYABLE_H

// Explicitly disallow the use of compiler-generated functions you do not want.
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    void operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif // WEBSERVER_BASE_NONCOPYABLE_H