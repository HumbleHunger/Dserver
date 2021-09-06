#include "ThreadPool.h"

namespace DJX
{

Threadpool::Threadpool(int numThread, int maxsize)
 : running_(false),
   queue_(maxsize),
   numThread_(numThread)
{
}

Threadpool::~Threadpool()
{
	if (running_)
	{
		stop();
	}
}

void Threadpool::start()
{
	assert(!running_);
	running_ = true;
	threads_.reserve(numThread_);
	for (int i = 0; i < numThread_; ++i)
	{
		threads_.emplace_back(new DJX::Thread(std::bind(&Threadpool::runInThread, this)));
		threads_[i]->start();
	}
}

void Threadpool::stop()
{
	running_ = false;
}
size_t Threadpool::queueSize() const
{
	return queue_.size();
}
// 往任务队列添加任务
void Threadpool::run(Task f)
{
	queue_.put(f);
}
// 从任务队列中取任务
Threadpool::Task Threadpool::take()
{
	Task ret = queue_.take();
	return ret;
}

void Threadpool::runInThread()
{
	while (running_)
	{
		Task task(take());
		if (task)
		{
			task();
		}
	}
	// 异常处理
}

} // namespace DJX