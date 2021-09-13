#ifndef DJX_ThreadPool_H
#define DJX_ThreadPool_H

#include "Thread.h"
#include "BlockingQueue.h"

#include <vector>

namespace DJX
{

class ThreadPool : noncopyable
{
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
	DJX::BlockingQueue<Task> queue_;
};
	
} // namespace DJX

#endif