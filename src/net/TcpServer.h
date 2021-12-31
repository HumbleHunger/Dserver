#ifndef DJX_TCPSERVER_H
#define DJX_TCPSERVER_H

#include "../base/Atomic.h"
#include "TcpConnection.h"
#include "../base/Mutex.h"

#include <map>

namespace DJX
{

namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
	TcpServer(EventLoop* loop, const InetAddress& listenAddr);
	~TcpServer();
/* 主要接口 */
	// 设置IO线程池的线程数目
	void setThreadNum(int numThreads);

	void start();

	// 用户注册的connectioncallback应该处理链接与断开，在TcpConnection链接与断开的时候都会调用
	void setConnectionCallback(const ConnectionCallback& cb)
	{ connectionCallback_ = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ messageCallback_ = cb; }

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ writeCompleteCallback_ = cb; }
/* 提供私有成员的访问 */
	// IO线程池
	std::shared_ptr<EventLoopThreadPool> IOthreadPool()
	{ return IOthreadPool_; }

	const string& ipPort() const { return ipPort_; }

	EventLoop* getLoop() const { return loop_; }

private:
	typedef std::map<TcpConnection*, TcpConnectionPtr> TcpConnectionMap;
	// 注册到acceptor中的回调函数，对新链接进行初始化（创建新链接，注册回调函数等）
	void newConnection(EventLoop* loop, int sockfd, const InetAddress& peerAddr);
	// 被下层类TcpConnection回调，处理链接关闭
	void removeConnection(const TcpConnectionPtr& conn);
	/// Not thread safe, but in loop
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	InetAddress listenAddr_;

	const string ipPort_;

	EventLoop* loop_;

	MutexLock mutex_;
	TcpConnectionMap connections_;
	
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