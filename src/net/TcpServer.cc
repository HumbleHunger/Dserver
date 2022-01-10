#include "TcpServer.h"

#include "../base/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"

#include <iostream>

using namespace DJX;
using namespace DJX::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
	:	listenAddr_(listenAddr),
		loop_(loop),
		ipPort_(listenAddr.toIpPort()),
		IOthreadPool_(new EventLoopThreadPool())
{
}

TcpServer::~TcpServer()
{
	LOG_TRACE << "TcpServer::~TcpServer " << " destructing";
}

void TcpServer::setThreadNum(int numThreads)
{
	assert( 0 <= numThreads);
	IOthreadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
	if (started_.getAndSet(1) == 0)
	{
		IOthreadPool_->start();
	}
	// 往每个loop插入一个acceptor
	for ( auto& loop : IOthreadPool_->getAllLoops() )
	{
		// 创建Acceptor
		std::unique_ptr<Acceptor> acceptor(new Acceptor(loop, listenAddr_, true));
		acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, loop, std::placeholders::_1, std::placeholders::_2));

		// 将Acceptor的所有权交给IO线程中的loop
		loop->setAcceptor(std::move(acceptor));
	}
}

void TcpServer::newConnection(EventLoop* loop, int sockfd, const InetAddress& peerAddr)
{
	LOG_INFO << "TcpServer::newConnection : EventLoop " <<  loop << " accept new connection from " << peerAddr.toIpPort();
	InetAddress localAddr(socketOps::getLocalAddr(sockfd));
	// 新建一个Tcponnection
	TcpConnectionPtr conn(new TcpConnection(loop,
	  																			sockfd,
                                        	localAddr,
                                        	peerAddr));
	// 设置Tcpconnection的各种回调函数
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	// 将Tcpconnection保存到TcpServer中
	/*{
		MutexLockGuard guard(mutex_);
		connections_[conn.get()] = conn;
	}*/
	
	// 将新建的Tcpconnection的channel加入到所属的IO线程loop中的Poller中关注
	loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

// 被下层Tcpconection::handleclose调用，删除链接
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	LOG_INFO << "TcpServer::removeConnectionInLoop - connection " << conn.get();
	/*{
		MutexLockGuard guard(mutex_);
		size_t n = connections_.erase(conn.get());
		(void)n;
		assert(n == 1);
	}*/
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}