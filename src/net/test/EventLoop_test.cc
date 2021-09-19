#include "../EventLoop.h"
#include "../../base/Thread.h"
#include "../../base/Logging.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace DJX;
using namespace DJX::net;

void callback()
{
	printf("callback(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
}

void threadFunc(EventLoop* loop)
{
	for (int i = 0; i < 10; ++i)
	{
		sleep(1);
		loop->runInLoop(std::bind(&callback));
		loop->wakeup();
	}
	loop->quit();
}

int main()
{
	Logger::setLogLevel(Logger::LogLevel::TRACE);
	printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  	assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
	EventLoop loop;
	assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

	Thread thread(std::bind(&threadFunc, &loop));
	thread.start();

	loop.loop();
}