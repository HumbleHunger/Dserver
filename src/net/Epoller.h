#ifndef DJX_POLLER_H
#define DJX_POLLER_H

#include <map>
#include <vector>

#include "../base/Timestamp.h"
#include "EventLoop.h"
#include "../base/noncopyable.h"

struct epoll_event;

namespace DJX
{
namespace net
{

class Channel;

class EPollPoller : noncopyable
{
public:
	typedef std::vector<Channel*> ChannelList;

	EPollPoller(EventLoop* loop);
	~EPollPoller();
/* 基本接口 */
	// 给loop提供的接口
	Timestamp poll(int timeoutMs, ChannelList* activeChannels);
	// 注册与删除事件
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);

	void assertInLoopThread() const
	{
	    ownerLoop_->assertInLoopThread();
	}
private:
	typedef std::map<int, Channel*> ChannelMap;
	typedef std::vector<struct epoll_event> EventList;
	// 初始化监听队列大小
	static const int kInitEventListSize = 16;
	// 返回就绪事件
	void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;
	// 更新channel的监听事件
	void update(int operation, Channel* channel);
/* 核心内容 */
	// fd，channel的map
	ChannelMap channels_;
	// 返回的事件列表
	EventList events_;
	// epollfd
	int epollfd_;
	// 返回的活动事件
	// 所属loop
	EventLoop* ownerLoop_;
};


} // namespace net
} // namespace DJX

#endif