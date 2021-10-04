#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

using namespace DJX;
using namespace DJX::net;

EventLoopThread::EventLoopThread(const InetAddress& listenAddr)
    :   exiting_(false),
        listenAddr_(listenAddr),
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
    EventLoop loop;

    // 设置loop_
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    Acceptor acceptor(&loop, listenAddr_, true);
    acceptor.listen();
    loop.loop();

    MutexLockGuard lock(mutex_);
    loop_ = NULL;
}
// IO线程对象拥有者线程调用
EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
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