#ifndef DJX_BUFFER_H
#define DJX_BUFFER_H

#include "../base/copyable.h"

#include <algorithm>
#include <string>
#include <vector>

#include <assert.h>
#include <string.h>

using std::string;

namespace DJX
{
namespace net
{

class Buffer : public DJX::copyable
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize)
		:	buffer_(kCheapPrepend + initialSize),
			readerIndex_(kCheapPrepend),
			writerIndex_(kCheapPrepend)
	{ }

	void swap(Buffer& rhs)
	{
		buffer_.swap(rhs.buffer_);
		std::swap(readerIndex_,rhs.readerIndex_);
		std::swap(writerIndex_,rhs.readerIndex_);
	}
/* 信息接口 */
	size_t readableBytes() const
	{
		return writerIndex_ - readerIndex_;
	}

	size_t writableBytes() const
	{
		return buffer_.size() - writerIndex_;
	}
	
	size_t prependableBytes() const
	{
		return readerIndex_;
	}
	// 返回第一个可读的char
	const char* peek() const
	{
		return begin() + readerIndex_;
	}

	char* beginWrite()
	{ 
		return begin() + writerIndex_;
	}

	const char* beginWrite() const
	{
		return begin() + writerIndex_;
	}


/* 基本接口 */

/* 取出数据接口 */
	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
	    if (len < readableBytes())
		{
	    	readerIndex_ += len;
		}
	    else
	    {
	    	retrieveAll();
		}
	}

	void retrieveUntil(const char* end)
	{
	    assert(peek() <= end);
	    assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveAll()
	{
	    readerIndex_ = kCheapPrepend;
	    writerIndex_ = kCheapPrepend;
	}
	// 将取回数据作为string返回
	string retrieveAllAsString()
	{
	    return retrieveAsString(readableBytes());
	}

	string retrieveAsString(size_t len)
	{
	    assert(len <= readableBytes());
	    string result(peek(), len);
	    retrieve(len);
	    return result;
	}

/* append接口 */
	void append(const char* /*restrict*/ data, size_t len)
	{
	    // 确保空间充足
	    ensureWritableBytes(len);
	    // 写入到Buffer中
	    std::copy(data, data+len, beginWrite());
	    // 调整写位置
	    hasWritten(len);
	}

/* read接口 */
	ssize_t readFd(int fd, int* savedErrno);

/* prepend接口 */
	void prepend(const void* /*restrict*/ data, size_t len)
	{
	    assert(len <= prependableBytes());
	    readerIndex_ -= len;
	    const char* d = static_cast<const char*>(data);
	    std::copy(d, d+len, begin()+readerIndex_);
	}

/* shrink接口 */
	// 提供更改shrink buffer大小的接口
	void shrink(size_t reserve)
	{
	    Buffer other;
	    other.ensureWritableBytes(readableBytes()+reserve);
	    other.append(peek(), readableBytes());
	    swap(other);
	}

private:
	char* begin()
	{ return &*buffer_.begin(); }

	const char* begin() const
	{ return &*buffer_.begin(); }

	// 确保缓冲区可写空间>=len，如果不足则扩充
	void ensureWritableBytes(size_t len)
	{
    	if (writableBytes() < len)
    	{
	    	makeSpace(len);
	    }
	    assert(writableBytes() >= len);
	}
	// 调整writerIndex
	void hasWritten(size_t len)
	{
	    assert(len <= writableBytes());
	    writerIndex_ += len;
	}

	void unwrite(size_t len)
	{
	    assert(len <= readableBytes());
	    writerIndex_ -= len;
	}

	void makeSpace(size_t len)
	{
	    // 判断prepend的空间加上空的空间是否足够写入数据
	    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
	    {
	    	// 如果不能则差多少增长多少空间
	    	buffer_.resize(writerIndex_+len);
	    }
	    // 如果可以则将数据迁移
	    else
	    {
	    	// move readable data to the front, make space inside buffer
	    	assert(kCheapPrepend < readerIndex_);
	    	size_t readable = readableBytes();
	    	std::copy(begin()+readerIndex_,
					  begin()+writerIndex_,
	      	          begin()+kCheapPrepend);
	    	readerIndex_ = kCheapPrepend;
	    	writerIndex_ = readerIndex_ + readable;
	    	assert(readable == readableBytes());
	    }
	}

	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
	
	static const char kCRLF[];
};

} // namespace net
} // namespcae DJX

#endif