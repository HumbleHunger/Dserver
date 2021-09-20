#ifndef DJX_TIMERID_H
#define DJX_TIMERID_H

#include "../base/copyable.h"
#include "Timer.h"

namespace DJX
{
namespace net
{

class Timer;
// Timer的标识类，不直接以Timer*作为标识。而重新封装TimerId使封装性更好
class TimerId : public DJX::copyable
{
public:
	TimerId()
		:	timer_(NULL),
			sequence_(0)
	{ }
	
	TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
	{ }

	friend class TimerQueue;
private:
	Timer* timer_;
	// 定时器序号，为什么不用TimerStamp因为TimerStamp会变化
	int64_t sequence_;
};

} // namespace net
} // namespace DJX

#endif