#ifndef DJX_TIMER_H
#define DJX_TIMER_H

#include "../base/Atomic.h"
#include "../base/Timestamp.h"
#include "Callbacks.h"

namespace DJX
{
namespace net
{
class Timer : noncopyable
{
public:

private:
	// 定时器回调函数
	const TimerCallback callback_;


};
} // namespace net
} // namespace DJX

#endif