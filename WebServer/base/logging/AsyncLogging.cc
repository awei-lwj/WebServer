#include "AsyncLogging.h"
#include "LogFile.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>

AsyncLogging::AsyncLogging(std::string logFileName, int flushInterval)
    : flushInterval_t(flushInterval),
      running(false),
      baseName(logFileName),
      thread_t(std::bind(&AsyncLogging::threadFunction, this), "Logging"),
      mutex_t(),
      condition_t(mutex_t),
      currentBuffer(new Buffer),
      prepareBuffer(new Buffer),
      buffers(),
      latch_t(1)
{
    assert(logFileName.size() > 1);
    currentBuffer->bzero();
    prepareBuffer->bzero();
    buffers.reserve(32);
}

// Leading end
void AsyncLogging::append(const char *logline, int len)
{
    MutexLockGuard lock(mutex_t);
    if (currentBuffer->avail() > len)
    {
        currentBuffer->append(logline, len); // most common case: buffer is not full,copy data here
    }
    else
    {
        buffers.push_back(currentBuffer); // buffer is full,push it,and find next spare buffer
        currentBuffer.reset();
        if (prepareBuffer) // if there is a spare buffer,using it
        {
            currentBuffer = std::move(prepareBuffer);
        }
        else // if everything is full,allocate a new buffer
        {
            currentBuffer.reset(new Buffer); // Rarely happens
        }
        currentBuffer->append(logline, len);
        condition_t.notify();
    }
}

// Backing end
void AsyncLogging::threadFunction()
{
    assert(running == true);

    latch_t.countDown();
    LogFile output(baseName);

    // 用作临界区中是否交换
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);

    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;

    buffersToWrite.reserve(32);

    while (running)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        // swap out what need to be written ,keep CS short
        {
            MutexLockGuard lock(mutex_t);

            // 等待条件触发
            if (buffers.empty())
            {
                condition_t.waitForSeconds(flushInterval_t);
            }
            buffers.push_back(currentBuffer);
            currentBuffer.reset();

            //先将当前缓冲（currentBuffer）移入 buffers，并立刻将空闲的newBuffer1移为当前缓冲。 
            // 注意这整段代码位于临界区之内，因此不会有任何race condition。
            currentBuffer = std::move(newBuffer1);
            buffersToWrite.swap(buffers);
            if (!prepareBuffer)
            {
                prepareBuffer = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // output bufferToWrite to file
        if (buffersToWrite.size() > 48)
        {
            buffersToWrite.erase(buffersToWrite.begin(), buffersToWrite.end());
        }

        for (size_t i = 0; i < buffersToWrite.size(); ++i)
        {
            output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
        }

        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        // re-fill newBuffer1 and newBuffer2
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

    } // end while

    output.flush();
}