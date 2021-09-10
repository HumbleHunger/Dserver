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

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
	typedef LogStream self;
public:
	typedef detail::FixedBuffer<>
	FixedBuffer()
	: cur_(data_)
	{}
	~FixedBuffer() = default;
	// 往缓冲区输入
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
	// 返回缓冲区的原始指针
	const char* data() const { return data_; }
  	int length() const { return static_cast<int>(cur_ - data_); }	
	// 返回当前可写入的位置
	char* current() { return cur_; }
	// 返回可写入的字节数
	int avail() const { return static_cast<int>(end() - cur_); }
	void add(size_t len) { cur_ += len; }
	// 只重置cur_指针
	void reset() { cur_ = data_; }
	// 清空缓冲区
	void bzero() { memZero(data_, sizeof data_); }
	// 转成String
	string toString() const { return string(data_, length()); }

private:
	const char* end() { return data_ + sizeof data_; }
	
	char data_[SIZE];
	char* cur_;
};
} // namespace detail
class LogStream : noncopyable
{
	typedef LogStream self;
public:
	typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;
	self& operator<<(bool v)
  	{
  		buffer_.append(v ? "1" : "0", 1);
    	return *this;
  	}

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* str)
  {
    if (str)
    {
      buffer_.append(str, strlen(str));
    }
    else
    {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str)
  {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const std::string& v)
  {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

	void append(const char* data, int len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void resetBuffer() { buffer_.reset(); }

private:
	// 检查空间是否足够
	void staticCheck();
	Buffer buffer_;
	// 格式化整数
	template<typename T>
	void formatInteger(T);

	static const int kMaxNumericSize = 32;
};

class Fmt // : noncopyable
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}
} // namespace DJX


#endif