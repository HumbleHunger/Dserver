#ifndef DJX_TIMERQUEUE_H
#define DJX_TIMERQUEUE_H

#include <set>
#include <vector>

#include "../base/Mutex.h"
#include "../base/Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace DJX
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{

public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();

  // 可以跨线程调用,内部使用runInloop
	TimerId addTimer(TimerCallback cb,
					 Timestamp when,
					 double interval);

	void cancel(TimerId timerId);

private:
	// 使用set来存储，方便搜索与删除
	typedef std::pair<Timestamp, Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

	// 只可能在所属IO线程中调用，所以不加锁
	void addTimerInLoop(Timer* timer);
	void cancelInLoop(TimerId timerId);
	
	// 处理定时事件的入口函数
	void handleRead();
	// 返回所有超时的定时器列表
	std::vector<Entry> getExpired(Timestamp now);
	// 重置超时的定时器
	void reset(const std::vector<Entry>& expired, Timestamp now);

	bool insert(Timer* timer);

/* 核心内容 */
	// 时间轮所属loop
	EventLoop* loop_;

	// 底层的时间轮文件描述符
	const int timerfd_;
	// 时间轮的通道
	Channel timerfdChannel_;
	
	// 按到期时间排序的定时器列表，方便插入insert
	TimerList timers_;

	// 按对象地址排序的定时器列表，方便取消cancel
	ActiveTimerSet activeTimers_;
	// 标志位
	bool callingExpiredTimers_; /* atomic */
	// 待取消的定时器集合
	ActiveTimerSet cancelingTimers_;
};


} // namespace net
} // namespace DJX

#endif