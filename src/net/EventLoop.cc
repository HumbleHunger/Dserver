#include "EventLoop.h"
#include "Channel.h"
#include "Epoller.h"
#include "SocketOps.h"
#include "TimerQueue.h"

#include "../base/Logging.h"
#include "../base/Mutex.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace DJX
{
namespace net
{
/* 此线程中的loop */
__thread EventLoop* t_loopInThisThread = 0;
// 每次poll等待的最大时间
const int kPollTimeMs = 10000;

// eventfd用于线程间通信，用来实现IO线程间的唤醒
int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
	    LOG_SYSERR << "Failed in eventfd";
	    abort();
	}
	return evtfd;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop()
	:	looping_(false),
		quit_(false),
		eventHandling_(false),
		callingPendingFunctors_(false),
		threadId_(CurrentThread::tid()),
		poller_(new EPollPoller(this)),
		timerQueue_(new TimerQueue(this)),
		wakeupFd_(createEventfd()),
		wakeupChannel_(new Channel(this, wakeupFd_)),
		currentActiveChannel_(NULL)
{
	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
	// 如果线程中已有loop则出现错误
	if (t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
		<< " exists in this thread " << threadId_;
	}
	else
	{
		t_loopInThisThread = this;
	}
	// 设置wakeup的回调函数
	wakeupChannel_->setReadCallback(
		std::bind(&EventLoop::handleRead, this));
	// 打开wakeup的读通道
	wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
	<< " destructs in thread " << CurrentThread::tid();

	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	// wakeupfd由loop管理
	::close(wakeupFd_);
	t_loopInThisThread = NULL;
}
/* 基本接口 */
	// 开始loop
void EventLoop::loop()
{
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	LOG_TRACE << "EventLoop " << this << " start looping";

	while (!quit_)
	{
		activeChannels_.clear();
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		if (Logger::logLevel() <= Logger::TRACE)
		{
			printActiveChannels();
		}
		eventHandling_ = true;
		// 遍历就绪事件列表，处理事件
		for (Channel* channel : activeChannels_)
		{
			currentActiveChannel_ = channel;
			// 调用channel的处理事件接口
			currentActiveChannel_->handleEvent(pollReturnTime_);
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		// 执行额外任务
		doPendingFunctors();
	}

	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
}

// 退出loop，可由其他线程调用
void EventLoop::quit()
{
	quit_ = true;
	// 如果被其他线程调用，则唤醒loop所在线程
	if (!isInLoopThread())
	{
		wakeup();
	}
}

/* 设置事件接口 */
// 在Poller中注册删除或修改关注的事件（Channel）,必须在loop所在线程调用
void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_)
	{
		assert(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	poller_->removeChannel(channel);
}

// 检查是否存在
bool EventLoop::hasChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return poller_->hasChannel(channel);
}

/* 为了执行其它线程分配的任务 */
	// 在IO线程中执行某个回调函数，使得函数可以跨线程调用
void EventLoop::runInLoop(Functor cb)
{
	if (isInLoopThread())
	{
		cb();
	}
	else
	{
		queueInLoop(std::move(cb));
	}
} 
// 可跨线程调用
void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}
	// 如果被非loop所在线程调用或loop线程正在处理pending任务则通过wakeupfd唤醒，使得任务可被快速执行
	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
}

size_t EventLoop::queueSize() const
{
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

// 唤醒poller,一般添加任务时调用
void EventLoop::wakeup()
{
	// 通过往wakefd中写入一个字节的数据来实现read唤醒
	uint64_t one = 1;
	ssize_t n = socketOps::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
	    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
	}
}

void EventLoop::abortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
			  << " was created in threadId_ = " << threadId_
			  << ", current thread id = " <<  CurrentThread::tid();
}

// WakefdChannel的回调函数
void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = socketOps::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
	}
}

// 执行待处理函数
void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	{
		MutexLockGuard lcok(mutex_);
		functors.swap(pendingFunctors_);
	}

	for (const Functor& functor : functors)
	{
		functor();
	}

	callingPendingFunctors_ = false;

}
	
void EventLoop::printActiveChannels() const
{
	for (const Channel* channel : activeChannels_)
	{
	    LOG_TRACE << "{" << channel->reventsToString() << "} ";
	}
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
	return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), delay));
	return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
	Timestamp time(addTime(Timestamp::now(), interval));
	return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
	return timerQueue_->cancel(timerId);
}

} // namespace net
} // namespace DJX