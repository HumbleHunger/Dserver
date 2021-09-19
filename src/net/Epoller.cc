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

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

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
	LOG_TRACE << "listen fd total count " << channels_.size();

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
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		// 将返回的事件类型添加到channel
		channel->set_revents(events_[i].events);
		// 将channel注册到Eventloop中
		activeChannels->push_back(channel);
	}
}

// 在poller中更新通道或者监听事件类型(通常被loop调用)
void EPollPoller::updateChannel(Channel* channel)
{
	EPollPoller::assertInLoopThread();
	// 获取index的值，如果小于0则为新channel
	const int index = channel->index();
	LOG_TRACE << "fd = " << channel->fd()
		<< " events = " << channel->events() << " index = " << index;
	// 添加channel
	if (index == kNew || index == kDeleted)
	{
		int fd = channel->fd();
		// 在poller中注册channel
		channels_[fd] = channel;
		channel->set_index(kAdded);
		// 添加到epoll的监听队列
		update(EPOLL_CTL_ADD, channel);
	}
	// 监听事件类型的修改
	else
	{
		int fd = channel->fd();
		if (channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else
		{
			// 更改在poller中关注的事件类型
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EPollPoller::removeChannel(Channel* channel)
{
	assertInLoopThread();
	int fd = channel->fd();
	LOG_TRACE << "fd = " << fd;
	
	assert(channels_.find(fd) != channels_.end());
	assert(channels_[fd] == channel);
	assert(channel->isNoneEvent());
	
	int index = channel->index();
	assert(index == kAdded || index == kDeleted);
	size_t n = channels_.erase(fd);
	assert(n  == 1);

	if (index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}

bool EPollPoller::hasChannel(Channel* channel) const
{
  assertInLoopThread();
  ChannelMap::const_iterator it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

// 对epoll操作的封装。被updateChannel或remove调用，更新epoll中的注册事件
void EPollPoller::update(int op, Channel* channel)
{
	struct epoll_event event;
	memset(&event, 0, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	LOG_TRACE << "epoll_ctl op = " << operationToString(op)
    << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
	if (::epoll_ctl(epollfd_, op, fd, &event) < 0)
	{
    	if (op == EPOLL_CTL_DEL)
    	{
    		LOG_SYSERR << "epoll_ctl op =" << operationToString(op) << " fd =" << fd;
    	}
    	else
    	{
    		LOG_SYSFATAL << "epoll_ctl op =" << operationToString(op) << " fd =" << fd;
    	}
	}
}

const char* EPollPoller::operationToString(int op)
{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}

} // namespace net
} // namespace DJX

