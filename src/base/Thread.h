#ifndef DJX_THREAD_H
#define DJX_THREAD_H

#include "CountDownLatch.h"
#include "Atomic.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace DJX
{

class Thread : noncopyable
{
public:
	typedef std::function<void ()> ThreadFunc;

	explicit Thread(ThreadFunc);

	~Thread();

	void start();

	bool started() const { return started_; }
  	
	static int numCreated() { return numCreated_.get(); }

private:
	// 线程入口函数
	void *startThread(void* obj);
	// 执行回调函数
	void runInThread();

	bool started_;
	bool joined_;
	pthread_t pthreadId_;
	// 线程在计算机中的唯一标识
	pid_t tid_;
	// 线程回调函数
  	ThreadFunc func_;
	CountDownLatch latch_;
	// Thread类所有
	static AtomicInt32 numCreated_;
};

}
#endif