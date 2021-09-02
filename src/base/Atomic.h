#ifndef DJX_ATOMIC_H
#define DJX_ATOMIC_H

#include "noncopyable.h"

#include <stdint.h>

namespace DJX
{

namespace detail
{
template<typename T>
class AtomicIntegerT : noncopyable
{
public:
  	AtomicIntegerT()
    : value_(0)
	{
  	}
	// 原子比较操作，返回value的值
  	T get()
  	{
    	// in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
    	return __sync_val_compare_and_swap(&value_, 0, 0);
  	}
	//返回value的值，再加上x
  	T getAndAdd(T x)
  	{
    	// in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
    	return __sync_fetch_and_add(&value_, x);
  	}
	//先加上x，再返回value的值
  	T addAndGet(T x)
  	{
    	return getAndAdd(x) + x;
  	}
	//自增
  	T incrementAndGet()
  	{
    	return addAndGet(1);
  	}
	//自减
  	T decrementAndGet()
	{
    	return addAndGet(-1);
  	}
	//先获取再加
  	void add(T x)
  	{
    	getAndAdd(x);
  	}
	// 自增
  	void increment()
  	{
    	incrementAndGet();
  	}
	// 自减
  	void decrement()
  	{
		decrementAndGet();
  	}
	//先获取值在设值
  	T getAndSet(T newValue)
	{
    // in gcc >= 4.7: __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST)
    	return __sync_lock_test_and_set(&value_, newValue);
  	}

private:
	// 每次使用它的时候必须地址中读取
	volatile T value_;
};

}

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

}

#endif