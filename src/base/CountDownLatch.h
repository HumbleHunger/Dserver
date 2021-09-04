#ifndef DJX_COUNTDOWNLATCH_H
#define DJX_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

namespace DJX
{

class CountDownLatch : noncopyable
{
public:
	explicit CountDownLatch(int count);

	void wait();

	void countDown();

	int getCount() const;

private:
	// 使mutex在const成员函数中也能被修改
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};

}
#endif