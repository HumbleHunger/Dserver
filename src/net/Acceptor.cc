#include "Acceptor.h"

#include "../base/Logging.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace DJX;
using namespace DJX::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
	:	loop_(loop),
		// 创建socket
		acceptSocket_(socketOps::createNonblockingOrDie(listenAddr.family())),
		acceptChannel_(loop, acceptSocket_.fd()),
		listening_(false)
		//idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reuseport);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();
}

void Acceptor::listen()
{
	loop_->assertInLoopThread();
	listening_ = true;
	acceptSocket_.listen();
	// 打开acceptor的读通道
	acceptChannel_.enableReading();
	LOG_INFO << "Thread " << CurrentThread::tid() << ": Acceptor of EventLoop " << loop_ << " is listening";
}

void Acceptor::handleRead()
{
	loop_->assertInLoopThread();

	InetAddress peerAddr;

	int connfd = acceptSocket_.accept(&peerAddr);
	if (connfd >= 0)
	{
		if (newConnectionCallback_)
		{
			newConnectionCallback_(connfd, peerAddr);
		}
		else
		{
			LOG_DEBUG << " 因未设置newConnectionCallback connfd " << connfd << " 将被关闭 ";
			socketOps::close(connfd);
		}
	}
	else
	{
		LOG_SYSERR << "in Acceptor::handleRead";
	}
}