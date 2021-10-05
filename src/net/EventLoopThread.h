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
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();

	bool exiting_;

	EventLoop* loop_;
	// 线程
	Thread thread_;
	// 同步控制loop在线程中生成后注册loop_
	MutexLock mutex_;
	Condition cond_;
};

}
}

#endif