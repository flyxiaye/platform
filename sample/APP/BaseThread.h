#ifndef BASETHREAD_H
#define BASETHREAD_H
extern "C" {
#include "ak_thread.h"
}

class BaseThread
{
public:
	BaseThread();;
	BaseThread(int stack_size, int priority);
	virtual ~BaseThread();
	void start();
	void wait();
	void wait(long time);
	void post();
	virtual void* run(void* arg) = 0;

private:
	ak_pthread_t thread_id;
	int stack_size;
	int priority;
	ak_sem_t sem;
};


#endif // !BASETHREAD_H
