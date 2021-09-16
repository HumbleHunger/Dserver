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

	const struct sockaddr* getSockAddr() const { return socketOps::sockaddr_cast(&addr_); }

	

private:
	struct sockaddr_in addr_;
};

} // namespace net
} // namespace DJX

#endif