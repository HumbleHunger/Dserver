#include "../ThreadPool.h"

#include <iostream>
#include <string>

void func(char* str)
{
	std::cout << str << std::endl;
}

void check(const DJX::Threadpool* tp)
{
	while (1)
	{
		std::cout << "queuesize is " << tp->queueSize() << std::endl;
	}
}

int main()
{
	DJX::Threadpool pool(8, 10);
	pool.start();
	for (int i = 0; i < 1000; ++i)
	{
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", i);
		pool.run(std::bind(&func, buf));
	}
}