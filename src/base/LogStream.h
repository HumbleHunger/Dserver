#ifndef DJX_LOGSTREAM_H
#define	DJX_LOGSTREAM_H

#include "noncopyable.h"
#include <string.h>
#include <assert.h>

namespace DJX
{
// SIZE是非类型参数
namespace detail
{
template<int SIZE>
class FixedBuffer : noncopyable
{
public:
	FixedBuffer()
	: cur_(data_)
	{}
	~FixedBuffer() = default;
	void append(const char* buf, size_t len)
	{
		// 如果有足够空间放入缓冲区
		if (static_cast<size_t>(avail()) > len)
		{
			memcpy(cur_, buf, len);
			// 移动cur_指针
			cur_ += len;
		}
	}

	const char* data() const { return data_; }
  	int length() const { return static_cast<int>(cur_ - data_); }	
	char* current() { return cur_; }

	int avail() const { return static_cast<int>(end() - cur_); }
	void add(size_t len) { cur_ += len; }
	// 只重置cur_指针
	void reset() { cur_ = data_; }
	// 清空缓冲区
	void bzero() { memZero(data_, sizeof data_); }
	// 装成String
	string toString() const { return string(data_, length()); }

private:
	const char* end() { return data_ + sizeof data_; }
	
	char data_[SIZE];
	char* cur_;
};
} // namespace detail
class LogStream
{
public:

private:


};

} // namespace DJX


#endif