#include <stdint.h>

/**
 * @brief
 * __thread是GCC内置的线程局部存储设施（thread local storage）。 它的实现非常高效，比pthread_key_t快很多，
 *
 * thread使用规则27：只能用于修饰POD类型，不能修饰class类型， 因为无法自动调用构造函数和析构函数。__thread可以用于修饰全局变 量、
 * 函数内的静态变量，但是不能用于修饰函数的局部变量或者class的 普通成员变量。
 *
 * __thread变量是每个线程有一份独立实体，各个线程的变量值互不 干扰。
 */

namespace CurrentThread
{
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char *t_threadName;

    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    // For logging
    inline const char *tidString()
    {
        return t_tidString;
    }

    // For logging
    inline int tidStringLength()
    {
        return t_tidStringLength;
    }

    inline const char *name()
    {
        return t_threadName;
    }

    bool isMainThread();

}