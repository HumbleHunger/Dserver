#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

using namespace DJX;
using namespace DJX::net;

EventLoopThread::EventLoopThread()
    :   exiting_(false),
        loop_(NULL),
        mutex_(),
        cond_(mutex_),
        thread_(std::bind(&EventLoopThread::threadFunc, this))
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != NULL)
    {
        loop_->quit();
        thread_.join();
    }
}
// IO线程执行
void EventLoopThread::threadFunc()
{
    // EnentLoop是栈上对象
    EventLoop loop;

    // 设置loop_
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();

    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}
// IO线程对象拥有者线程调用
EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    // 内部调用threadFunc函数，创建新线程调用threadFunc并在栈上创建loop对象
    thread_.start();

    EventLoop* loop = NULL;
    // 使用条件变量，等待threadFunc函数设置loop_，即loop被创建
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}