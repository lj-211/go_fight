#ifndef THREAD_SYNC_H
#define THREAD_SYNC_H

#include "go_define.h"

#include <pthread.h>

namespace thread {

class Mutex {
public:
	Mutex() {
		pthread_mutex_init(&mutex_, NULL);
	}

	~Mutex() {
		pthread_mutex_destroy(&mutex_);
	}

	pthread_mutex_t& GetMutex() {
		return mutex_;
	}

	void Hold() {
		pthread_mutex_lock(&mutex_);
	}

	void Release() {
		pthread_mutex_unlock(&mutex_);
	}

private:
	pthread_mutex_t mutex_;	

private:
	DISALLOW_COPY_AND_ASSIGN(Mutex);
};

class AutoLockMutex {
public:
	AutoLockMutex(Mutex* mutex) {
		mutex_ = mutex;
		pthread_mutex_lock(&(mutex_->GetMutex()));
	}
	~AutoLockMutex() {
		pthread_mutex_unlock(&(mutex_->GetMutex()));
	}

private:
	Mutex* mutex_;
private:
	DISALLOW_COPY_AND_ASSIGN(AutoLockMutex);
};

};

#endif
