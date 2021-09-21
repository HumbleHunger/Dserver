#ifndef DJX_ACCEPTOR_H
#define DJX_ACCEPTOR_H

#include <functional>

#include "Channel.h"
#include "Socket.h"

namespace DJX
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
// 服务端监听套接字的封装
class Acceptor : noncopyable
{
public:
	typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
	~Acceptor();

	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{ newConnectionCallback_ = cb; }

	void listen();

	bool listening() const { return listening_; }

private:
	// 监听socket的回调函数，accept新链接
	void handleRead();

	EventLoop* loop_;
	
	Socket acceptSocket_;
	Channel acceptChannel_;

	// 处理新链接的回调函数
	NewConnectionCallback newConnectionCallback_;

	bool listening_;
	//int idleFd_;
};

} // namespace net
} // namespcae DJX

#endif