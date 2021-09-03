#include "CountDownLatch.h"

using namespace DJX;

CountDownLatch::CountDownLatch(int count)
	: mutex_(),
	  condition_(mutex_),
	  count_(count)
{	
}

void CountDownLatch::wait()
{
	MutexLockGuard(mutex_);
	while (count > 0)
	{
		condition_.wait();
	}
}

void CountDownLatch::countDown()
{
	MutexLockGuard(mutex_);
	--count;
	if (count == 0)
	{
		condition_.notifyAll();
	}
}

int CountDownLatch::getCount() const
{
	MutexLockGuard(mutex_);
	return count_;
}