#ifndef DJX_CHANNEL_H
#define DJX_CHANNEL_H

#include "../base/noncopyable.h"
#include "../base/Timestamp.h"

#include <functional>
#include <memory>

namespace DJX
{
namespace net
{

class EventLoop;

class Channel : noncopyable
{
public:
	typedef std::function<void()> EventCallback;
	typedef std::function<void(Timestamp)> ReadEventCallback;

	Channel(EventLoop* loop, int fd);
	~Channel();
	// 处理事件的统一接口
	void handleEvent(Timestamp receiveTime);
	// 设置事件回调函数接口
	void setReadCallback(ReadEventCallback cb)
	{ readCallback_ = std::move(cb); }
	void setWriteCallback(EventCallback cb)
	{ writeCallback_ = std::move(cb); }
	void setCloseCallback(EventCallback cb)
	{ closeCallback_ = std::move(cb); }
	void setErrorCallback(EventCallback cb)
	{ errorCallback_ = std::move(cb); }
	
	// 对监听事件类型注册的开关
	void enableReading() { events_ |= kReadEvent; update(); }
	void disableReading() { events_ &= ~kReadEvent; update(); }
	void enableWriting() { events_ |= kWriteEvent; update(); }
	void disableWriting() { events_ &= ~kWriteEvent; update(); }
	void disableAll() { events_ = kNoneEvent; update(); }
  	
	// 判断监听的IO事件类型
  	bool isWriting() const { return events_ & kWriteEvent; }
  	bool isReading() const { return events_ & kReadEvent; }

private:

/* 核心内容 */
	// Channel所属的loop
	// 用于区别返回IO事件类型
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	const int fd_;
	// 需要监听的IO事件类型，往poller中注册的事件
	int events_;
	// 就绪的IO事件类型
	int revents_;
	// Channel的回调函数
	ReadEventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

}

} // namespace net
} // namespace DJX

#endif