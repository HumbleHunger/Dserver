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

class EventLoopThreadPool : noncopyable
{
public:
	EventLoopThreadPool(const InetAddress& listenAddr);
	~EventLoopThreadPool() = default;

	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	void start();

	bool started() const
	{ return started_; }

	std::vector<EventLoop*> getAllLoops();
private:
	InetAddress listenAddr_;
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