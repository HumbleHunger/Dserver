#include "EventLoop.h"
#include "Channel.h"
#include "Epoller.h"
#include "SocketsOps.h"
//#include "TimerQueue.h"

#include "../base/Logging.h"
#include "../base/Mutex.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace DJX
{
namespace net
{
// eventfd用于线程间通信，用来实现IO线程间的唤醒
int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
	    LOG_SYSERR << "Failed in eventfd";
	    abort();
	}
	return evtfd;
}

} // namespace net
} // namespace DJX