#ifndef WEBSERVER_BASE_THREAD_ATOMIC_H
#define WEBSERVER_BASE_THREAD_ATOMIC_H

// 实现线程之间的原子操作

#include "../Noncopyable.h"
#include "stdint.h"

template <typename T>
class AtomicIntegerT : public noncopyable
{
public:
    AtomicIntegerT() : value(0) {}

    T get() const
    {
        return __sync_val_compare_and_swap(const_cast<volatile T *>(&value), 0, 0);
    }

    T getAndAdd(T x)
    {
        return __sync_fetch_and_add(&value, x);
    }

    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
    }

    T incrementAndGet()
    {
        return addAndGet(1);
    }

private:
    volatile T value;
};

//定义原子操作类型
typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

#endif // WEBSERVER_BASE_THREAD_ATOMIC_H