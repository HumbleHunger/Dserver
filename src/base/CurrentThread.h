#ifndef DJX_CURRENTTHREAD_H
#define DJX_CURRENTTHREAD_H

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace DJX
{
	// 线程局部数据
	namespace CurrentThread
	{
		// 线程真实pid的缓存，为提高获取tid的效率
		extern __thread int t_cachedTid;
 		extern __thread char t_tidString[32];
  		extern __thread int t_tidStringLength;
		
		// 获取tid,第一次调用会缓存tid到cache
		inline int tid()
		{
			if (t_cachedTid == 0)
			{
	 		   	t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
	 		   	t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
			}
			return t_cachedTid;
		}

		inline const char* tidString()
		{
			return t_tidString;
		}

		inline int tidStringLength()
		{
			return t_tidStringLength;
		}
	} // namespace CurrentThread
} // namespace DJX

#endif