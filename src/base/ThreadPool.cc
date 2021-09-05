#include "ThreadPool.h"

namespace DJX
{

Threadpool::Threadpool()
 : mutex_(),
   notEmpty_(mutex_),
   notFull_(mutex_),
   maxQueueSize_(0),
   running_(false)
{
}

Threadpool::~Threadpool()
{
	if (running_)
	{
		stop();
	}
}

void Threadpool::start(int numThreads)
{
	running_ = true;
	threads_.reserve(numThreads);
	for (int i = 0; i < numThreads; ++i)
	{
		threads_.emplace_back(new DJX::Thread(std::bind(&runInThread, this)));
		threads_[i]->start();
	}
}

void Threadpool::stop()
{
	running_ = false;
}
size_t Threadpool::queueSize() const
{

}
// 往任务队列添加任务
void Threadpool::run(Task f)
{
	MutexLockGuard lock(mutex_);
	while (isFull() && running_)
	{
		notFull_.wait();
	}
	queue_.put(f);
}

void Threadpool::runInThread()
{
	while (running_)
	{
		Task task(queue_.take());
		if (task)
		{
			task();
		}
	}
	// 异常处理
}

} // namespace DJX