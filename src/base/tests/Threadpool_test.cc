#include "../ThreadPool.h"

#include <iostream>
#include <string>

int sum;
int sum1;
DJX::MutexLock mutex_;
DJX::MutexLock mutex_1;
void func(char* str)
{
	//std::cout << str << std::endl;
	DJX::MutexLockGuard lock(mutex_);
	++sum;
}
void func1(char* str)
{
	//std::cout << str << std::endl;
	DJX::MutexLockGuard lock(mutex_1);
	++sum1;
}

void check(const DJX::ThreadPool* tp)
{
	while (1)
	{
		std::cout << "queuesize is " << tp->queueSize() << std::endl;
	}
}

int main()
{
	{
	DJX::ThreadPool pool(8, 10);
	pool.start();
	for (int i = 0; i < 100000; ++i)
	{
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", i);
		pool.run(std::bind(&func, buf));
		pool.run(std::bind(&func1, buf));

	}
	sleep(3);
	}
	printf("sum = %d\n", sum);
	printf("sum1 = %d\n", sum1);
}