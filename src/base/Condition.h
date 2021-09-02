#ifndef DJX_CONDITION_H
#define DJX_CONDITION_H

#include "Mutex.h"
#include <pthread.h>

class Condition : noncopyable
{
public:
	explicit Condition(MutexLock& mutex) : mutex_(mutex_)
	{
		pthread_cond_init(&cond_,NULL);
	}

	~Condition()
	{
		pthread_cond_destroy(&cond_);
	}

	void wait()
	{
		pthread_cond_wait(&cond_, mutex_.getPthreadCond());
	}

	void notify()
	{
		pthread_cond_signal(&cond_);
	}

private:
	pthread_cond_t cond_;
	MutexLock& mutex_;
};

#endif