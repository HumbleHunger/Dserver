#include "../Thread.h"

#include <iostream>

void thread1()
{
	std::cout << "this is thread1" << std::endl;
}

void thread2()
{
	std::cout << "this is thread2" << std::endl;
}

void thread3()
{
	std::cout << "this is thread3" << std::endl;
}

bool test(DJX::Thread& obj)
{
	std::cout << "started is " << obj.started() << std::endl;
	obj.start();
	std::cout << "started is " << obj.started() << std::endl;
	std::cout << "tid is " << obj.tid() << std::endl;
	std::cout << "numCreated is " << obj.numCreated() << std::endl;
	return true;
}

int main()
{
	DJX::Thread obj1(thread1);
	test(obj1);
	DJX::Thread obj2(thread2);
	test(obj2);
	DJX::Thread obj3(thread3);
	test(obj3);
}