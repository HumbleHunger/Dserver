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
	if (started_)
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

void Thread::runInThread()
{
	tid_ = CurrentThread::tid();
	latch_.countDown();
	func_();
	// 异常处理
}
} // namespacr DJX		