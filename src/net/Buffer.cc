#include "Buffer.h"

#include "SocketOps.h"

#include <errno.h>
#include <sys/uio.h>

using namespace DJX;
using namespace DJX::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

// 从socket中读取数据
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;

	const ssize_t n = socketOps::readv(fd, vec, iovcnt);
	if (n < 0)
	{
		*savedErrno = errno;
	}
	// 如果数据只写入到Buffer则只调整写位置
	else if (static_cast<size_t>(n) <= writable)
	{
	    writerIndex_ += n;
	}
	// 如果Buffer不够写入到了extrabuf中，则将extrabuf的数据append到Buffer中
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return n;
}