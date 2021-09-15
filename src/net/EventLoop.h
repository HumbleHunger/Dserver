#ifndef DJX_EVENTLOOP_H
#define DJX_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "../base/Mutex.h"
#include "../base/CurrentThread.h"
#include "../base/Timestamp.h"
#include "Callbacks.h"
//#include "TimerId.h"

namespace DJX
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
	// 待处理的回调函数形式
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();
/* 基本接口 */
	// 开始loop
	void loop();
	// 退出loop，可由其他线程调用
	void quit();

	Timestamp poolReturnTime() const { return poolReturnTime_; }

/* 设置事件接口 */
	// 在Poller中注册删除或修改关注的事件（Channel）
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	// 检查是否存在
	bool hasChannel(Channel* channel);

/* 定时器任务 
  	// 定时器设置接口
  	TimerId runAt(Timestamp time, TimerCallback cb);
  	// 执行任务在某段时间后
  	TimerId runAfter(double delay, TimerCallback cb);
  	// 间隔执行的定时任务设置接口
  	TimerId runEvery(double interval, TimerCallback cb);
  	// 取消定时任务
  	void cancel(TimerId timerId);
*/

/* 为了执行其它线程分配的任务 */
	// 在IO线程中执行某个回调函数，使得函数可以跨线程调用
	void runInLoop(Functor cb);
	// 
	void queueInLoop(Functor cb);

	size_t queueSize() const;
	// 唤醒poller,一般当执行紧急任务时调用
	void wakeup();
	
/* 断言在loopThread中 */
	void assertInLoopThread()
	{
    	if (!isInLoopThread())
	    {
    		abortNotInLoopThread();
    	}
  	}
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	
	bool eventHandling() const { return eventHandling_; }

private:
	void abortNotInLoopThread();
	// WakefdChannel的回调函数
	void handleRead();  // waked up
	// 执行待处理函数
	void doPendingFunctors();
	
	void printActiveChannels() const; // DEBUG
	
	typedef std::vector<Channel*> ChannelList;
	
/* 标志位 */
	bool looping_;
	std::atomic<bool> quit_;
	bool eventHandling_;
	bool callingPendingFunctors_;
	// loop所属线程ID
	const pid_t threadId_;
	// pool返回的时间
	Timestamp poolReturnTime_;
	
/* 核心内容 */	
	// Poller
	std::unique_ptr<Poller> poller_;
	
	// 时间轮,处理定时事件
	std::unique_ptr<TimerQueue> timerQueue_;

	// 处理一般事件（socket事件）
	// poller返回的Channel的列表vector
	ChannelList activeChannels_;
	// 当前正在处理的Channel
	Channel* currentActiveChannel_;
	
/* 为了处理紧急任务和IO线程池的关闭退出 */
	// 当在非所属IO线程中调用quit函数时，作为唤醒通道使阻塞在poll阶段的IO线程被唤醒
	int wakeupFd_;
	std::unique_ptr<Channel> wakeupChannel_;

/* 待处理的回调函数列表，使其他线程可往loop线程添加任务，可实现loop线程间通信 */
	std::vector<Functor> pendingFunctors_;

};
} // namespace net
} // namespace DJX

#endif
