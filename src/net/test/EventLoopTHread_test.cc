#include "../EventLoopThread.h"
#include "../EventLoop.h"
#include "../../base/Thread.h"
#include "../../base/CountDownLatch.h"
#include "../InetAddress.h"
#include "../../base/Logging.h"
#include <stdio.h>
#include <unistd.h>

using namespace DJX;
using namespace DJX::net;

void print(EventLoop* p = NULL)
{
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

void quit(EventLoop* p)
{
  print(p);
  p->quit();
}

int main()
{
	//Logger::setLogLevel(Logger::LogLevel::TRACE);
	print();

	InetAddress addr("127.0.0.1", 8000);
	{
		EventLoopThread thr1(addr);  // never start
	}

  {
  // dtor calls quit()
  EventLoopThread thr2(addr);
  EventLoop* loop = thr2.startLoop();
  loop->runInLoop(std::bind(print, loop));
  sleep(1);
  }

  {
  // quit() before dtor
  EventLoopThread thr3(addr);
  EventLoop* loop = thr3.startLoop();
  loop->runInLoop(std::bind(&quit, loop));
  sleep(1);
  }
}