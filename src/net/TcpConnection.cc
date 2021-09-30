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
		peerAddr_(peerAddr),
		highWaterMark_(64*1024*1024)
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

void TcpConnection::send(const string& message)
{
	if (state_ == kConnected)
	{
		// 如果在所属IO线程中则直接调用sendinloop
		if (loop_->isInLoopThread())
		{
			sendInLoop(message);
		}
		// 其他线程调用，则将sendinloop函数注册到所属loop的pendingfunctor中
		else
		{
			void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
			loop_->runInLoop(std::bind(fp, this, message));
		}
	}
}

void TcpConnection::send(const void* message, int len)
{
	string str(static_cast<const char*>(message), len);
	send(std::move(str));
}

void TcpConnection::send(Buffer& message)
{
	if (state_ == kConnected)
	{
		// 如果在当前IO线程则直接调用sendinloop
		if (loop_->isInLoopThread())
		{
			sendInLoop(message.peek(), message.readableBytes());
			message.retrieveAll();
		}
		// 其他线程调用
		else
		{
			void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
			loop_->runInLoop(std::bind(fp, this, message.retrieveAllAsString()));
		}
	}
}

void TcpConnection::sendInLoop(const string& message)
{
	sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if (state_ == kDisconnected)
	{
		LOG_WARN << "disconnected, give up writing";
    	return;
	}
	// 如果未关注write事件且outputbuffer为空，则尝试直接往socket写数据
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
	{
		nwrote = socketOps::write(channel_->fd(), data, len);
		if (nwrote >= 0)
		{
			remaining -= nwrote;
			// 如果写完则注册回调函数到pendingfunctor
			if (remaining == 0 && writeCompleteCallback_)
			{
				loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
			}
		}
		else
		{
			nwrote = 0;
			if (errno != EAGAIN)
			{
				LOG_SYSERR << "TcpConnection::sendInLoop";
        		if (errno == EPIPE || errno == ECONNRESET)
        		{
        			faultError = true;
        		}
			}
		}	
	}
	// 如果正在关注write事件或者如果前面一次性没有写完(即有未写完的数据)，将未写入到内核缓冲区的数据添加到output buffer，并开启关注套接字的写事件
	assert(remaining <= len);
	if (!faultError && remaining > 0)
	{
		size_t oldLen = outputBuffer_.readableBytes();
		// 如果output buffer的大小超过hignWaterMark_（高水位标），回调函数
		if (oldLen + remaining >=  highWaterMark_ && highWaterMarkCallback_)
		{
			loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(), oldLen + remaining));
		}
		// 将剩余的数据保存到buffer
		outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);

		if (!channel_->isWriting())
		{
			channel_->enableWriting();
		}
	}
}

void TcpConnection::startRead()
{
	loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
	loop_->assertInLoopThread();
	if (!reading_ || !channel_->isReading())
	{
		channel_->enableReading();
		reading_ = true;
	}
}

void TcpConnection::stopRead()
{
	loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void TcpConnection::shutdown()
{
	if (state_ == kConnected)
	{
		// 调整状态为正在断开链接状态
		setState(kDisconnecting);
		loop_->runInloop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  // 如果未关注write事件则关闭fd的写端，如果仍在关注写事件则仅仅把链接状态改为正在关闭链接
  if (!channel_->isWriting())
  {
    socket_->shutdownWrite();
  }
}