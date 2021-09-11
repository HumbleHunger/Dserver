#include "Thread.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <assert.h>

namespace DJX
{
namespace detail
{
	// 线程入口函数
	void* startThread(void* obj)
	{
		Thread* thread_ = static_cast<Thread*>(obj);
		thread_->runInThread();
		return NULL;		
	}
} // namespace DJX::detail

AtomicInt32 Thread::numCreated_;

Thread::Thread(ThreadFunc func)
	: started_(false),
	  joined_(false),
	  pthreadId_(0),
	  tid_(0),
	  func_(std::move(func)),
	  latch_(1)
{
	numCreated_.incrementAndGet();	
}

Thread::~Thread()
{
	// 回收资源
	if (started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

void Thread::start()
{
  	assert(!started_);
	started_ = true;
  	if (pthread_create(&pthreadId_, NULL, &detail::startThread, this))
	{
		//log_error
	}
	// 等待线程成功执行再返回
	else
	{
		latch_.wait();
	}
}

int Thread::join()
{	
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}

void Thread::runInThread()
{
	tid_ = CurrentThread::tid();
	latch_.countDown();
	try
	{
		func_();
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
} // namespacr DJX		