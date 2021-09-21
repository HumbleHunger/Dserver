#include "InetAddress.h"

#include "../base/Logging.h"
#include "Endian.h"
#include "SocketOps.h"

#include <netdb.h>
#include <netinet/in.h>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

namespace DJX
{
namespace net
{

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
	memset(&addr_, 0, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = socketOps::hostToNetwork32(ip);
    addr_.sin_port = socketOps::hostToNetwork16(port);
}

InetAddress::InetAddress(const char* ip, uint16_t port)
{
	memset(&addr_, 0, sizeof addr_);
	socketOps::fromIpPort(ip, port, &addr_);
}

std::string InetAddress::toIp() const
{
	char buf[64] = "";
	socketOps::toIp(buf, sizeof buf, getSockAddr());
	return buf;
}

std::string InetAddress::toIpPort() const
{
	char buf[64] = "";
	socketOps::toIpPort(buf, sizeof buf, getSockAddr());
	return buf;
}

uint16_t InetAddress::port() const
{
	return socketOps::networkToHost16(portNetEndian());
}



} // namespace net

} // namespace DJX