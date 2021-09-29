#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketOps.h"

#include "../base/Logging.h"

#include <errno.h>
#include <functional>

using namespace DJX;
using namespace DJX::net;

TcpConnection::TcpConnection(EventLoop* loop,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
	:	loop_(loop),
		state_(kConnecting),
		reading_(true),
		socket_(new Socket(sockfd)),
		channel_(new Channel(loop, sockfd)),
		localAddr_(localAddr),
		peerAddr_(peerAddr)
		//hignWaterMark_(64*1024*1024)
{
	// 设置channel的各类回调函数
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

	LOG_DEBUG << "TcpConnection::ctor at " << this
            << " fd = " << sockfd;
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
	LOG_DEBUG << "TcpConnection::dtor at " << this
            << " fd=" << channel_->fd()
            << " state=" << stateToString();
	assert(state_ == kDisconnected);
}

void TcpConnection::send(string&& message)
{
	if (state_ == kConnected)
	{
		// 如果在所属IO线程中则直接调用sendinloop
		if (loop_->isInLoopThread())
		{
			sendInLoop(std::forward<string>(message));
		}
		// 其他线程调用，则将sendinloop函数注册到所属loop的pendingfunctor中
	}
}
void TcpConnection::send(const void* message, int len)
{

}
void TcpConnection::send(Buffer&& message)
{

}