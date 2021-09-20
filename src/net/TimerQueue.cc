#include "TimerQueue.h"

#include "../base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace DJX
{
namespace net
{
namespace detail
{

int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
    								TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
	{
	    LOG_SYSFATAL << "Failed in timerfd_create";
	}
	return timerfd;
}

// 计算超时时刻与当前时间的时间差
struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
    	microseconds / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(
    	(microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
	return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
  	uint64_t howmany;
  	ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  	LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  	if (n != sizeof howmany)
  	{
  		LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
	}
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  // 重置时间轮的唤醒时间
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    LOG_SYSERR << "timerfd_settime()";
  }
}

} // namespace detail

TimerQueue::TimerQueue(EventLoop* loop)
	:	loop_(loop),
    timerfd_(detail::createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false)
{
	timerfdChannel_.setReadCallback(
    	std::bind(&TimerQueue::handleRead, this));
	timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	// 删除timerfdChannel
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	::close(timerfd_);

	// delete调Timer
	for (const Entry& timer : timers_)
	{
		delete timer.second;
	}
}
// add 和 cancel可以跨线程调用
TimerId TimerQueue::addTimer(TimerCallback cb,
                             Timestamp when,
                             double interval)
{
	// 创建timer
	Timer* timer = new Timer(std::move(cb), when, interval);
	loop_->runInLoop(
		std::bind(&TimerQueue::addTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
	loop_->runInLoop(
		std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
	loop_->assertInLoopThread();
	// 将timer插入到时间轮中
	bool earliestChanged = insert(timer);
	// 如果最早到期的定时器发生改变，则重置时间轮的唤醒时间
	if (earliestChanged)
	{
		detail::resetTimerfd(timerfd_, timer->expiration());
	}
}

bool TimerQueue::insert(Timer* timer)
{
	loop_->assertInLoopThread();
	// timer_和activeTimers_大小必须一样
	assert(timers_.size() == activeTimers_.size());
	
	bool earliestChanged = false;
	// 获取定时器的超时时间和最早的超时时间比较
	Timestamp when = timer->expiration();
	TimerList::iterator it = timers_.begin();
	
	// 如果时间轮为空或者插入的定时器超时时间比已有的早
	if (it == timers_.end() || when < it->first)
	{
		earliestChanged = true;
	}
	// 插入到timers_中
	{
		std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
		assert(result.second);
	}
	// 插入到activeTimers_中
	{
		std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
	    assert(result.second);
	}
	
	assert(timers_.size() == activeTimers_.size());
	return earliestChanged;
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
	loop_->assertInLoopThread();
	assert(timers_.size() == activeTimers_.size());

	ActiveTimer timer(timerId.timer_, timerId.sequence_);
	ActiveTimerSet::iterator it = activeTimers_.find(timer);
	// 如果定时器仍未处理，则直接删除 （ISSUES:如果定时器已经超时但未处理怎么办？）
	if (it != activeTimers_.end())
	{
		size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
		assert(n == 1);
		delete it->first;  // ISSUES:如果之前传出去的TimerId继续被使用是否有内存问题
		activeTimers_.erase(it);
	}
	// 如果loop线程正在处理定时事件，则先加入到取消列表中  (ISSUES:似乎不可能发生？)
	else if (callingExpiredTimers_)
	{
	    cancelingTimers_.insert(timer);
	}

	assert(timers_.size() == activeTimers_.size());
}

// 定时器事件处理入口
void TimerQueue::handleRead()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());
	// 仅仅读出事件，避免一直触发
	detail::readTimerfd(timerfd_, now);
	// 获取所有的超时定时器
	std::vector<Entry> expired = getExpired(now);

	callingExpiredTimers_ = true;
	cancelingTimers_.clear();
	// 遍历回调定时器处理函数
	for (const Entry& it : expired)
	{
    	it.second->run();
	}
	callingExpiredTimers_ = false;
	// 重置超时的定时器，如果是重复定时器则需要重启
	reset(expired, now);
}
// 获取超时的定时器
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
	assert(timers_.size() == activeTimers_.size());
	
	std::vector<Entry> expired;
	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	
	// 返回第一个未超时的Timer迭代器
	TimerList::iterator end = timers_.lower_bound(sentry);
	// 将超时的Timer复制到expired中
	std::copy(timers_.begin(), end, back_inserter(expired));
	// 将超时的Timer在  timers_删除
	timers_.erase(timers_.begin(), end);
	// 将超时的Timer在activeTimers_中删除
  	for (const Entry& it : expired)
	{
	    ActiveTimer timer(it.second, it.second->sequence());
	    size_t n = activeTimers_.erase(timer);
	    assert(n == 1);
	}

	assert(timers_.size() == activeTimers_.size());
	// rvo性能优化，expired不会被用于拷贝构造出一个新的对象返回，而是直接返回expired
	return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
	Timestamp nextExpire;

	for (const Entry& it : expired)
	{
		ActiveTimer timer(it.second, it.second->sequence());
		// 如果定时器是重复定时器并且不在待取消的定时器列表中则重置
    	if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
    	{
    		it.second->restart(now);
    		insert(it.second);
		}
		// 删除到期的定时器以及定时器取消列表中的定时器
    	else
    	{
    		delete it.second;
    	}
	}
	// 重置时间轮的下一次唤醒时间
	if (!timers_.empty())
	{
		nextExpire = timers_.begin()->second->expiration();
	}
	
	if (nextExpire.valid())
	{
    	detail::resetTimerfd(timerfd_, nextExpire);
	}
}

} // namespace net
} // namespace DJX