#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <stdio.h>
#include <iostream>


namespace DJX
{
AsyncLogging::AsyncLogging(const std::string& basename,
                           off_t rollSize,
                           int flushInterval)
    :   flushInterval_(flushInterval),
        running_(false),
        basename_(basename),
        rollSize_(rollSize),
        thread_(std::bind(&AsyncLogging::threadFunc, this)),
        latch_(1),
        mutex_(),
        cond_(mutex_),
        // 创建两块缓冲区
        currentBuffer_(new Buffer),
        nextBuffer_(new Buffer),
        buffers_()
{
    std::cout << "start asyncLogging" << std::endl;
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

// 由前端线程调用
void AsyncLogging::append(const char* logline, int len)
{
    // 锁住缓冲区，防止多个前端线程同时往缓冲区写数据和与日志线程访问缓冲区互斥
    DJX::MutexLockGuard lock(mutex_);
    // 当缓冲区有足够空间储存数据时，直接写入
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    else
    {
        // 如果没有足够空间储存数据
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        // 如果前端写入数据太快，把两块缓冲区都写完，那么新分配缓冲区
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    // 创建日志文件
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        // 临界区
        {
            DJX::MutexLockGuard lock(mutex_);
            if (buffers_.empty())
            {
                cond_.waitForSeconds(flushInterval_);
            }
            // 1.将当前缓冲区push到buffers_中(日志线程被唤醒有两种情况，但都几乎意味着当前缓冲区有数据)
            buffers_.push_back(std::move(currentBuffer_));
            // 2.将空闲的newbuffer1设置为当前缓冲区
            currentBuffer_ = std::move(newBuffer1);
            // 3.buffers_与buffersToWrite（栈上对象）交换，这样之后的代码可以在临界区之外访问（相当于将buffers_的数据暂时储存）
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                // 确保前端始终有一个预备buffer可供调配，减少前端临界区分配内存的概率，缩短前端临界区
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());
        // 防止出现消息堆积问题
        // 前端一直发送日志消息，超过后端处理能力。造成数据在内存中堆积，严重时引发性能问题（可用内存不足）
        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            // 如果出现消息堆积问题，则丢掉多余的日志，以腾出内存，只保留两块缓冲区
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        //将日志信息输出
        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }

        // 将缓冲区内容输出后，只保留两块缓冲区以备用newBuffer使用
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }
        // 如果newBuffer为空则把buffers中的缓冲区给它（此时，buffers中缓冲区的内容已经写入到日志）
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        // 清空
        buffersToWrite.clear();
        // 手动刷新缓冲区
        output.flush();
    }
    output.flush();
}
} // namespace DJX