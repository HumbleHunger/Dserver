#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
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