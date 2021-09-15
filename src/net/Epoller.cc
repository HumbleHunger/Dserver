#include "Epoller.h"

#include "../base/Logging.h"
#include "Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace DJX
{
namespace net
{
EPollPoller::EPollPoller(EventLoop* loop)
	:	ownerLoop_(loop),
		epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    	events_(kInitEventListSize)
{
	if (epollfd_ < 0)
	{
	    LOG_SYSFATAL << "EPollPoller::EPollPoller";
	}
}

EPollPoller::~EPollPoller()
{
	// 使用RAII的手法管理epollfd
	::close(epollfd_);
}

// 将就绪的channel注册到activeChannels中
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
	LOG_TRACE << "fd total count" << channels_.size();

	int numEvents = ::epoll_wait(epollfd_,
								&*events_.begin(),
								static_cast<int>(events_.size()),
								timeoutMs);
	// 保存epoll_wait的错误值
	int savedErrno = errno;
	// 保存wait返回的时间
	Timestamp now(Timestamp::now());
	if (numEvents > 0)
	{
		LOG_TRACE << numEvents << "  events happend";
		// 返回事件channel
		fillActiveChannels(numEvents, activeChannels);
		// 如果所有事件都是活动事件，则扩大vector
		if (static_cast<size_t>(numEvents) == events_.size())
		{
			events_.resize(events_.size()*2);
		}
	}
	else if (numEvents == 0)
	{
		LOG_TRACE << " nothing happended ";
	}
	else
	{
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			LOG_SYSERR << "EPollPoller::poll()";
		}
	}
	return now;
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const
{
	
}

} // namespace net
} // namespace DJX

