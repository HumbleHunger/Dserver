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
	// 给前端现称调用
	void append(const char* logline, int len);

	void start()
	{
	}
	
	void stop()
	{
	}
private:
	// 日志线程入口函数
	void threadFunc();

	typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef BufferVector::value_type BufferPtr;
	// 刷新间隔
	const int flushInterval_;
	std::atomic<bool> running_;
	// 日志滚动大小
	const off_t rollSize_;
	Thread thread_;
	CountDownLatch latch_;
	// 锁住前后端通信的缓冲区
	MutexLock mutex_;
	Condition cond_;
	// 当前使用缓冲区的指针
	BufferPtr currentBuffer_;
	// 预备缓冲区指针
	BufferPtr nextBuffer_;
	BufferVector buffers_;

};

} // namespace DJX
#endif