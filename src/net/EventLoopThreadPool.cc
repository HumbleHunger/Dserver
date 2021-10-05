#include "EventLoopThreadPool.h"

#include "EventLoop.h"

#include <cassert>

using namespace DJX;
using namespace DJX::net;

EventLoopThreadPool::EventLoopThreadPool()
	:	started_(false),
		numThreads_(0)
{
}

// 由Tcpserver（在主IO线程中）调用
void EventLoopThreadPool::start()
{
	assert(!started_);

	started_ = true;

	for (int i = 0; i < numThreads_; ++i)
	{
		// 创建IO线程对象并保存到线程池的IO线程管理队列中
		EventLoopThread* t = new EventLoopThread();
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		// 开始IO线程，创建并返回IO线程的loop
		loops_.push_back(t->startLoop());
	}
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
	assert(started_);
	return loops_;
}