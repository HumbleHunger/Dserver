#ifndef DJX_BLOCKINGQUEUE_H
#define DJX_BLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <deque>
#include <assert.h>

namespace DJX
{
template<typename T>
class BlockingQueue : noncopyable
{
public:
	BlockingQueue()
	: mutex_(),
	  notEmpty_(mutex_),
	  queue_()
	{}
	// 左值和const右值
	void put(const T& x)
	{
		MutexLockGuard lock(mutex_);
		queue_.push_back(x);
		notEmpty_.notify();
	}
	// 绑定到非const右值
	void put(T&& x)
	{
		MutexLockGuard lock(mutex_);
		// 因为T的类型在此函数中不确定，所以使用std::move做转型
		queue_.push_back(std::move(x));
		notEmpty_.notify();
	}

	T take()
	{
		MutexLockGuard lock(mutex_);
		while (queue_.empty())
		{
			notEmpty_.wait();
		}
		T front(std::move(queue_.front()));
		queue_.pop_front();
		return front;
	}

	size_t size() const
	{
		MutexLockGuard lock(mutex_);
		return queue_.size();
	}

private:
	// mutable使const函数也能使用锁
	mutable MutexLock mutex_;
	Condition notEmpty_;
	std::deque<T> queue_;
};

} // namespace DJX

#endif