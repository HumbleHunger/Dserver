#ifndef DJX_TCPCONNECTION_H
#define DJX_TCPCONNECTION_H

#include "../base/noncopyable.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"

#include <memory>

#include <boost/any.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace DJX
{
namespace net
{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
	~TcpConnection();

/* 主要接口 */
	void startRead();
	void stopRead();

	void send(string&& message);
	void send(const void* message, int len);
	void send(Buffer&& message);

	void shutdown();

	void forceClose();
	// 当新链接出现时调用
	void connectEstablished();
	// 当链接销毁时调用
	void connectDestroyed(); 
/* 设置回调接口 */
	void setConnectionCallback(const ConnectionCallback& cb)
	{ connectionCallback_ = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ messageCallback_ = cb; }

	void setWriteCompleteCallback(const WriteCompleteCallback& cb)
	{ writeCompleteCallback_ = cb; }

//	void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
//	{ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }
/* 访问私有信息接口 */
	EventLoop* getLoop() const { return loop_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }

	Buffer* inputBuffer()
	{ return &inputBuffer_; }

	Buffer* outputBuffer()
	{ return &outputBuffer_; }

private:
	// 链接状态：析构设置未链接，构造时设置正在链接，已链接，正在关闭链接
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

	// 设置到Channel中的回调函数
	void handleRead(Timestamp receiveTime);
	void handleWrite();
	void handleClose();
	void handleError();

/* 核心内容 */
	// 所属loop
	EventLoop* loop_;
	// 链接状态
	StateE state_;
	// 底层socket
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
  
	const InetAddress localAddr_;
	const InetAddress peerAddr_;

	// 应用层的输入输出缓存区
	Buffer inputBuffer_;
	Buffer outputBuffer_;

/* 保存由上层类（TcpServer）注册的回调函数 */
	// 连接到来时的回调函数
	ConnectionCallback connectionCallback_;
	// 消息到来时的回调函数,被成员函数handleRead调用
	MessageCallback messageCallback_;
	// 消息写入完成时的回调函数
	WriteCompleteCallback writeCompleteCallback_;
	// 链接关闭时的回调函数
	CloseCallback closeCallback_;
/*
	// output buffer的高水位回调函数
	HighWaterMarkCallback highWaterMarkCallback_;
	CloseCallback closeCallback_;
	// output buffer的高水位数值
	size_t highWaterMark_;
*/
};



} // namespace net
} // namespace DJX

#endif