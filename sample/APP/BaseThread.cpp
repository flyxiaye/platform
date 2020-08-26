#include "BaseThread.h"
#include <functional>
#include <iostream>

void * test(void * arg){
	std::cout << "test thread" << std::endl;	
}

BaseThread::BaseThread(int stack_size, int priority) {
	this->stack_size = stack_size;
	this->priority = priority;
	ak_thread_sem_init(&sem, 0);
}

BaseThread::~BaseThread()
{
	// std::cout << "destroy thread\n";
	// ak_thread_join(thread_id);
	ak_thread_cancel(this->thread_id);
	ak_thread_sem_destroy(&sem);
}

void BaseThread::start(thread_func callback)
{
	ak_thread_create(&thread_id, callback, (void*)this, stack_size, priority); 
}

void BaseThread::wait()
{
	ak_thread_sem_wait(&sem);
}

void BaseThread::wait(long time)
{
	// Nanosecond
	struct timespec t = {0, time};
	ak_thread_sem_timedwait(&sem, &t);
}

void BaseThread::post()
{
	ak_thread_sem_post(&sem);
}

void BaseThread::stop()
{
	ak_thread_cancel(thread_id);
}

void BaseThread::join()
{
	ak_thread_join(thread_id);
}

static void * callback(void * arg);

class Test_thread :public BaseThread
{
public:
	Test_thread() { a = 0;};
	~Test_thread() {};
	void run(void){
		std::cout << "test thread" << a << std::endl;	
	}
	
private:
	int a;
};

static void * callback(void * arg)
{
	((Test_thread*)arg)->run();
}



void test_thread()
{
	Test_thread t;
	t.start(callback);
}

