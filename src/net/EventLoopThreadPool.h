#ifndef DJX_EVENTLOOPTHREADPOOL_H
#define DJX_EVENTLOOPTHREADPOOL_H

#include "../base/noncopyable.h"

#include "InetAddress.h"
#include "EventLoopThread.h"

#include <functional>
#include <memory>
#include <vector>


namespace DJX
{

namespace net
{

class EventLoop;

// 并非真正意义上的线程池，线程的主循环是eventloop。为了Tcpserver管理IO线程
class EventLoopThreadPool : noncopyable
{
public:
	EventLoopThreadPool();
	~EventLoopThreadPool() = default;

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();

	bool started() const
	{ return started_; }

	std::vector<EventLoop*> getAllLoops();
private:
	bool started_;
	int numThreads_;
	// IO线程管理队列
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	// IO线程的loop
	std::vector<EventLoop*> loops_;
};

} // namespace net
} // namespace DJX

#endif