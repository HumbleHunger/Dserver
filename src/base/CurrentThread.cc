#include "CurrentThread.h"

#include <stdlib.h>

namespace DJX
{
	// 定义CurrentThread的线程局部变量
	namespace CurrentThread
	{
		__thread int t_cachedTid = 0;
		__thread char t_tidString[32];
		__thread int t_tidStringLength = 6;

	} // namespace CurrentThread
} // namespace DJX