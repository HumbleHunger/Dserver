#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sstream>

#include <poll.h>

using std::string;

namespace DJX
{
namespace net
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
	:	loop_(loop),
		fd_(fd),
		events_(0),
		revents_(0),
		index_(-1),
		eventHandling_(false),
		addedToLoop_(false)
{
}

Channel::~Channel()
{
	assert(!eventHandling_);
	assert(!addedToLoop_);
	// TODO	
}

void Channel::handleEvent(Timestamp receiveTime)
{
	// 用以保存tied_提升后获取的智能指针，在链接关闭时，可以保证调用的Tcpconnection对象一直存在
	std::shared_ptr<void> guard;
	if (tied_)
	{
		guard = tie_.lock();
		if (!guard) 
		{
			LOG_WARN << "TcpConnection has dead in handleEvent which channel of fd = " << fd_;
			return;
		}
	}
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	// 链接中断
	if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
	{
    	LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
  		if (closeCallback_) closeCallback_();
  	}
	
	// 出现错误
	if (revents_ & (POLLERR | POLLNVAL))
	{
    	if (errorCallback_) errorCallback_();
	}

	// 可读事件
	if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
 	{
 	   if (readCallback_) readCallback_(receiveTime);
	}

	// 可写事件
	if (revents_ & POLLOUT)
	{
		if (writeCallback_) writeCallback_();
	}

	eventHandling_ = false;
}

void Channel::update()
{
	addedToLoop_ = true;
	// 调用loop的update函数
	loop_->updateChannel(this);
}

void Channel::remove()
{
	assert(isNoneEvent());
	addedToLoop_ = false;
	// 调用loop的remove函数
	loop_->removeChannel(this);
}

string Channel::reventsToString() const
{
  return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
  return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

} // namespace net
} // namespace JDX