#ifndef DJX_CHANNEL_H
#define DJX_CHANNEL_H

#include "../base/noncopyable.h"
#include "../base/Timestamp.h"

#include <functional>
#include <memory>

using std::string;

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

	// 在Poller中删除,调用先需确保events_ = 0，即未注册任何监听事件
	void remove();
/* 标志位判断 */
	bool isNoneEvent() const { return events_ == kNoneEvent; }
/* debug */
	string reventsToString() const;
	string eventsToString() const;

/* 提供对内部成员的访问 */
	int fd() const { return fd_; }
	int events() const { return events_; }
	void set_revents(int revt) { revents_ = revt; }
	EventLoop* ownerLoop() { return loop_; }

/* for poller */
  // 关于channel是否被添加到epoll中的标志位
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

private:
	static string eventsToString(int fd, int ev);
	// 在Eventloop和Poller中注册Channel
	void update();
/* 核心内容 */
	// Channel所属的loop
	EventLoop* loop_;
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
/* 标志位 */
	bool eventHandling_;
	bool addedToLoop_;
	// 标记channel是否是新链接以及其他状态
	int index_;
};

} // namespace net
} // namespace DJX

#endif