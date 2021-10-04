#ifndef DJX_TCPSERVER_H
#define DJX_TCPSERVER_H

#include "../base/Atomic.h"
#include "TcpConnection.h"

#include <map>

namespace DJX
{

namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

// 注册到acceptor中的回调函数，对新链接进行初始化（创建新链接，注册回调函数等）
void newConnection(int sockfd, const InetAddress& peerAddr);

class TcpServer : noncopyable
{
public:

private:
	const string ipPort_;
	std::shared_ptr<EventLoopThreadPool> IOthreadPool_;
	// 新链接时的回调函数,由用户注册
	ConnectionCallback connectionCallback_;
	// 有消息到来时的回调函数
	MessageCallback messageCallback_;
	// socket数据写入内核完成时的回调函数
  	WriteCompleteCallback writeCompleteCallback_;
	AtomicInt32 started_;
};
} // namespace net

} // namespace DJX

#endif