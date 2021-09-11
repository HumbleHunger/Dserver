#include "ThreadPool.h"

namespace DJX
{

ThreadPool::ThreadPool(int numThread, int maxsize)
 : running_(false),
   queue_(maxsize),
   numThread_(numThread)
{
}

ThreadPool::~ThreadPool()
{
	if (running_)
	{
		stop();
	}
}

void ThreadPool::start()
{
	assert(!running_);
	running_ = true;
	threads_.reserve(numThread_);
	// 将自身指针传给queue
	queue_.bind(this);
	for (int i = 0; i < numThread_; ++i)
	{
		threads_.emplace_back(new DJX::Thread(std::bind(&ThreadPool::runInThread, this)));
		threads_[i]->start();
	}
}

void ThreadPool::stop()
{
	queue_.notifyAll(true);

	for (auto& thr : threads_)
	{
		thr->join();
	}
}

size_t ThreadPool::queueSize() const
{
	return queue_.size();
}
// 往任务队列添加任务
void ThreadPool::run(Task f)
{
	queue_.put(f);
}
// 从任务队列中取任务
ThreadPool::Task ThreadPool::take()
{
	Task ret = nullptr;
	ret = queue_.take();
	return ret;
}

void ThreadPool::runInThread()
{
	try
	{
		while (running_)
		{
			Task task(take());
			if (task)
			{
				task();
			}
		}
	}
	catch (const Exception& ex)
	{
		fprintf(stderr, "Exception caught in Thread %d\n", CurrentThread::tid());
		fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
	}
	catch (const std::exception& ex)
	{
		fprintf(stderr, "exception caught in Thread %d\n", CurrentThread::tid());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
	}
	catch (...)
	{
		fprintf(stderr, "unknown exception caught in Thread %d\n", CurrentThread::tid());
        throw;
	}
	// 异常处理
}

} // namespace DJX