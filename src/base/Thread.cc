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
/*namespace detail
{*/
void* Thread::startThread(void* obj)
{
	Thread* thread_ = static_cast<Thread*>(obj);
	thread_->runInThread();
	return NULL;
}
//} // namespace DJX::detail

Thread::Thread(ThreadFunc func)
	: started_(false),
	  joined_(false),
	  pthreadId_(0),
	  tid_(0),
	  func_(std::move(func)),
	  latch_(1)
{	
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
  	if (pthread_create(&pthreadId_, NULL, &startThread, this)
	{
	}
}

void Thread::runInThread()
{
	latch_.countDown();
	func_();
	// 异常处理
}
} // namespacr DJX		