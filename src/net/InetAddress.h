#ifndef DJX_INETADDRESS_H
#define DJX_INETADDRESS_H

#include "../base/copyable.h"

#include <netinet/in.h>

namespace DJX
{
namespace net
{

namespace socketOps
{
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
}

class InetAddress : public copyable
{
public:
	// explicit InetAddress();
	explicit InetAddress(const struct sockaddr_in& addr)
    	:	addr_(addr)
	{ }

	InetAddress(const char* ip, uint16_t port);

	const struct sockaddr* getSockAddr() const { return socketOps::sockaddr_cast(&addr_); }
	void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

	sa_family_t family() const { return addr_.sin_family; }	
	std::string toIp() const;
	std::string toIpPort() const;
	uint16_t port() const;

	// 地址网络序
	uint32_t ipv4NetEndian() const;
	uint16_t portNetEndian() const { return addr_.sin_port; }

private:
	struct sockaddr_in addr_;
};

} // namespace net
} // namespace DJX

#endif