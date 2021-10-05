#include "../EventLoopThreadPool.h"
#include "../EventLoop.h"
#include "../../base/Thread.h"
#include "../../base/Logging.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

using namespace DJX;
using namespace DJX::net;

void print(EventLoop* p = NULL)
{
  printf("main(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

void init(EventLoop* p)
{
  printf("init(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

int main()
{
	//Logger::setLogLevel(Logger::LogLevel::TRACE);
	print();

  {
    EventLoopThreadPool model;
    model.setThreadNum(2);
    model.start();
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model;
    model.setThreadNum(1);
    model.start();
    std::vector<EventLoop*> Loops = model.getAllLoops();
    Loops[0]->runAfter(2, std::bind(print, Loops[0]));
    sleep(2);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model;
    model.setThreadNum(3);
    model.start();
    std::vector<EventLoop*> Loops = model.getAllLoops();
    Loops[0]->runAfter(2, std::bind(print, Loops[0]));
    sleep(3);
  }
  sleep(2);
}
