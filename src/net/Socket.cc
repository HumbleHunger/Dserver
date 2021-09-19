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
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}
	// 端口复用
void Socket::setReusePort(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		LOG_SYSERR << "SO_REUSEPORT failed.";
	}
}
	
void Socket::shutdownWrite()
{
	socketOps::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
				 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
				 &optval, static_cast<socklen_t>(sizeof optval));
}

} // namespace net
} // namespace DJX