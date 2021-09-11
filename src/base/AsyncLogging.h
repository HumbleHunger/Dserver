#ifndef DJX_ASYNCLOGGING_H
#define DJX_ASYNCLOGGING_H

#include "BlockingQueue.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"

#include <atomic>
#include <vector>

namespace DJX
{
class AsyncLogging : noncopyable
{
public:
	AsyncLogging(off_t rollSize_, int flushInterval_ = 3);
	~AsyncLogging()
	{
		if (running_)
		{
			stop();
		}
	}

	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop()
	{
		running_ = false;
		cond_.notify();
		thread_.join();
	}
private:
	// 日志线程入口函数
	void threadFunc();

	typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;
  	
	const int flushInterval_;
	std::atomic<bool> running_;
	const off_t rollSize_;
	Thread thread_;
	CountDownLatch latch_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;

};

} // namespace DJX
#endif