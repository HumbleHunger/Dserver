#ifndef DJX_NONCOPYABLE_H
#define DJX_NONCOPYABLE_H

namespace DJX
{

class noncopyable
{
public:
	// 使用delete关键字明确地禁止copying操作
	noncopyable(const noncopyable&) = delete;
	void operator=(const noncopyable&) = delete;

protected:
	noncopyable() {};
	~noncopyable() {};
};

}

#endif