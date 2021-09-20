#include "../EventLoop.h"
#include "../../base/Thread.h"
#include "../../base/Logging.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace DJX;
using namespace DJX::net;

EventLoop* g_loop;

void callback()
{
  printf("callback(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
}

void threadFunc()
{
 /* printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  loop.runEvery(1.0, callback);
  loop.loop();*/
  sleep(10);

}

int main()
{
	Logger::setLogLevel(Logger::LogLevel::TRACE);
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

  loop.runEvery(1.0, std::bind(&callback));
  loop.loop();
}