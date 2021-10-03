#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

using namespace DJX;
using namespace DJX::net;

EventLoopThread::EventLoopThread(const InetAddress& listenAddr)
    :   exiting_(false),
        listenAddr_(listenAddr),
        thread_(std::bind(&EventLoopThread::threadFunc, this))
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (thread_.started())
    {
        loop_->quit();
        thread_.join();
    }
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    loop_ = &loop;

    Acceptor acceptor(&loop, listenAddr_, true);
    acceptor.listen();
    loop.loop();

    loop_ = NULL;
}

void EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();
}