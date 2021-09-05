#ifndef DJX_THREADPOOL_H
#define DJX_THREADPOOL_H

#include "Thread.h"
#include "BlockingQueue.h"

#include <vector>

namespace DJX
{

class Threadpool : noncopyable
{
public:
	typedef std::function<void ()> Task;
	
	explicit Threadpool();
	~Threadpool();
	// 必须在调用start()之前调用
	void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
	void start(int numThreads);
	void stop();
	size_t queueSize() const;
	// 往任务队列添加任务
	void run(Task f);

private:
	bool isFull() const;
	// 工作线程的入口函数
	void runInThread();
	// 从
	Task take();

	bool running_;
	mutable MutexLock mutex_;
	Condition notEmpty_;
	Condition notFull_;
	// 任务队列的最大长度
	size_t maxQueueSize_;
	// 线程列表
	std::vector<std::unique_ptr<Thread>> threads_;
	// 阻塞队列
	BlockingQueue<Task> queue_;
};
	
} // namespace DJX

#endif