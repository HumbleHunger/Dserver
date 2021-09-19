#ifndef DJX_SOCKET_H
#define DJX_SOCKET_H

#include "../base/noncopyable.h"

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace DJX
{
namespace net
{

class InetAddress;

class Socket : noncopyable
{
public:
	explicit Socket(int sockfd)
		:	sockfd_(sockfd)
	{}

	~Socket();
/* 基础接口 */
	// 绑定地址
	void bindAddress(const InetAddress& localaddr);
	// 转为监听socket
  	void listen();
	// 接受链接
	int accept(InetAddress* peeraddr);
	// 地址复用
	void setReuseAddr(bool on);
	// 端口复用
	void setReusePort(bool on);
	
	void shutdownWrite();
	// 设置不使用nagle算法
	void setTcpNoDelay(bool on);
	// 开启keepAlive机制
	void setKeepAlive(bool on);

	int fd() const { return sockfd_; }

private:
	const int sockfd_;

};

} // namespace net
} // namespcae DJX
#endif