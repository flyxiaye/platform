#ifndef BASETHREAD_H
#define BASETHREAD_H
extern "C" {
#include "ak_thread.h"
}

class BaseThread
{
public:
	BaseThread() {
		BaseThread(ANYKA_THREAD_MIN_STACK_SIZE, -1);
	};
	BaseThread(int stack_size, int priority) {
		this->stack_size = stack_size;
		this->priority = priority;
	}
	virtual ~BaseThread() {};
	void start() { ak_thread_create(thread_id, run, nullptr, stack_size, priority); }
	virtual void* run(void* arg) = 0;

private:
	ak_pthread_t thread_id;
	int stack_size;
	int priority
};


#endif // !BASETHREAD_H
