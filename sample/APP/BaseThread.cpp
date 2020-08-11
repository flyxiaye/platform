#include "BaseThread.h"

BaseThread::BaseThread() {
	BaseThread(ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

BaseThread::BaseThread(int stack_size, int priority) {
	this->stack_size = stack_size;
	this->priority = priority;
	ak_thread_sem_init(sem, 0);
}

BaseThread::~BaseThread()
{
	ak_thread_sem_destroy(sem);
}

void BaseThread::start() 
{ 
	ak_thread_create(thread_id, run, nullptr, stack_size, priority); 
}

void BaseThread::wait()
{
	ak_thread_sem_wait(sem);
}

void BaseThread::wait(long time)
{
	// Nanosecond
	struct timespec t(0, time);
	ak_thread_sem_timedwait(sem, t);
}

void BaseThread::post()
{
	ak_thread_sem_post(sem);
}

