#ifndef DJX_MUTEX_H
#define DJX_MUTEX_H

#include "noncopyable.h"
#include <pthread.h>
#include <memory>

namespace DJX
{

class MutexLock : noncopyable
{
public:
	MutexLock()
	{
		pthread_mutex_init(&mutex_, NULL);
	}

	~MutexLock()
	{
		pthread_mutex_destroy(&mutex_);
	}

	void lock()
	{
		pthread_mutex_lock(&mutex_);
	}
	
	void unlock()
	{
		pthread_mutex_unlock(&mutex_);
	}

	pthread_mutex_t* getPthreadMutex() /* non-const */
  	{
    	return &mutex_;
	}

private:
	pthread_mutex_t mutex_;
};

class MutexLockGuard : noncopyable
{
public:
	explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock& mutex_;
};

}

#endif