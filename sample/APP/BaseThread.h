#ifndef BASETHREAD_H
#define BASETHREAD_H
extern "C" {
#include "ak_thread.h"
}

class BaseThread
{
public:
	BaseThread() {};
	virtual ~BaseThread();
	virtual void* thread_call_back(void* arg);
	void start();

private:
	ak_pthread_t thread_id;
};


#endif // !BASETHREAD_H
