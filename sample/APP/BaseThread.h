#ifndef BASETHREAD_H
#define BASETHREAD_H
#include <iostream>
extern "C" {
#include "ak_thread.h"
}

class BaseThread
{
public:
	// BaseThread();;
	BaseThread(int stack_size = ANYKA_THREAD_MIN_STACK_SIZE, int priority = -1);
	virtual ~BaseThread();
	void run(void) {};
	void start(thread_func callback);
	void start();
	void wait();
	void wait(long time);
	void post();
	void stop();
	// virtual void run(void * arg) { std::cout << "BaseThread\n"; };

private:
	ak_pthread_t thread_id;
	int stack_size;
	int priority;
	ak_sem_t sem;
};

void test_thread();

#endif // !BASETHREAD_H
