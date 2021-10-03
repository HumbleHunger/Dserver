#ifndef DJX_EVENTLOOPTHREAD_H
#define DJX_EVENTLOOPTHREAD_H

#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"

namespace DJX
{
namespace net
{
// IO线程封装类
class EventLoopThread : noncopyable
{
public:
	EventLoopThread(const InetAddress& listenAddr);
	~EventLoopThread();
	void startLoop();

	EventLoop* getLoop() { if (thread_.started()) return loop_; }
private:
	void threadFunc();

	bool exiting_;
	InetAddress listenAddr_;

	EventLoop* loop_;
	// 线程
	Thread thread_;
};

}
}

#endif