#include "Socket.h"

#include "../base/Logging.h"
#include "InetAddress.h"
#include "SocketOps.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // snprintf

namespace DJX
{
namespace net
{
Socket::~Socket()
{
	socketOps::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
  socketOps::bindOrDie(sockfd_, addr.getSockAddr());
}

// 转为监听socket
void Socket::listen()
{
	socketOps::listenOrDie(sockfd_);
}
	// 接受链接
int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	int connfd = socketOps::accept(sockfd_, &addr);
	if (connfd >= 0)
	{
		peeraddr->setSockAddrInet(addr);
	}
	return connfd;
}
	// 地址复用
void Socket::setReuseAddr(bool on)
{

}
	// 端口复用
void Socket::setReusePort(bool on)
{

}
	
void Socket::shutdownWrite()
{

}

void Socket::setTcpNoDelay(bool on)

void Socket::setKeepAlive(bool on)

}
}