#ifndef DJX_ThreadPool_H
#define DJX_ThreadPool_H

#include "Thread.h"
//#include "BlockingQueue.h"

#include <vector>
#include <deque>
#include <assert.h>

namespace DJX
{

class ThreadPool : noncopyable
{
template<typename T>
class BlockingQueue : noncopyable
{
public:
	BlockingQueue()
	: mutex_(),
	  notEmpty_(mutex_),
	  notFull_(mutex_),
	  queue_()
	{}

	BlockingQueue(int maxsize)
	: mutex_(),
	  notEmpty_(mutex_),
	  notFull_(mutex_),
	  queue_()
	{
		setMaxQueueSize(maxsize);
	}
	// 左值和const右值
	void put(const T& x)
	{
		MutexLockGuard lock(mutex_);
		while (isFull() && pool_->running_)
		{
			notFull_.wait();
		}
    	// 如果线程池已经stop则不添加，因为中间有解锁操作所以还应该判断
		if (!pool_->running_) return;
		queue_.push_back(x);
		notEmpty_.notify();
	}
	// 绑定到非const右值
	void put(T&& x)
	{
		MutexLockGuard lock(mutex_);
		// 因为T的类型在此函数中不确定，所以使用std::move做转型
		while (isFull() && pool_->running_)
		{
			notFull_.wait();
		}
    	// 如果线程池已经stop则不添加，因为中间有解锁操作所以还应该判断
		if (!pool_->running_) return;
		queue_.push_back(std::move(x));
		notEmpty_.notify();
	}

	T take()
	{
		MutexLockGuard lock(mutex_);
  		// 两个条件必须同时满足，不然无法关闭
		while (queue_.empty() && pool_->running_)
		{
			notEmpty_.wait();
		}
    	// 如果线程池已经stop则不获取，因为中间有解锁操作所以还应该判断
		if (!pool_->running_) return nullptr;
		T front(std::move(queue_.front()));
		queue_.pop_front();
		notFull_.notify();
		return front;
	}

	size_t size() const
	{
		MutexLockGuard lock(mutex_);
		return queue_.size();
	}
	// for ThreadPool
	void bind(ThreadPool* pool)
	{
		pool_ = pool;
	}

	void notEmpty_notifyAll()
	{
		MutexLockGuard lock(mutex_);
		notEmpty_.notifyAll();
	}
	
	void notFull_notifyAll()
	{
		MutexLockGuard lock(mutex_);
		notFull_.notifyAll();
	}

	void notifyAll(bool stop)
	{
		MutexLockGuard lock(mutex_);
		if (stop) pool_->running_ = false;
		notEmpty_.notifyAll();
		notFull_.notifyAll();
	}
private:
	bool isFull()
	{
		return queue_.size() >= maxQueueSize_;
	}
	void setMaxQueueSize(int maxsize) { maxQueueSize_ = maxsize; }
	// mutable使const函数也能使用锁
	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	size_t maxQueueSize_;
	std::deque<T> queue_;
	ThreadPool* pool_;
};
public:
	typedef std::function<void ()> Task;
	
	explicit ThreadPool(int numThread, int maxsize);
	~ThreadPool();
	void start();
	void stop();
	size_t queueSize() const;
	// 往任务队列添加任务
	void run(Task f);

	bool running_;
private:
	// 工作线程的入口函数
	void runInThread();
	Task take();

	int numThread_;
	// 任务队列的最大长度
	// 线程列表
	std::vector<std::unique_ptr<Thread>> threads_;
	// 阻塞队列
	BlockingQueue<Task> queue_;
};
	
} // namespace DJX

#endif