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

}
}