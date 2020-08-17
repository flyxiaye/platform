#ifndef DATABUFF_H
#define DATABUFF_H
extern "C" {
#include "ak_mem.h"
#include "ak_thread.h"
}


class DataBuffer
{
public:
	DataBuffer(int module_id=0, int size = 200 * 1024);
	~DataBuffer();
	int rb_write(unsigned char * src, int write_len);
	int rb_read(unsigned char * dst, int read_len, int* result);
	int rb_write(DataBuffer &df_src, int write_len);
	// int rb_read(DataBuffer &df_dst);
	// int rb_get_buffer_id() { return buffer_id; }
	int rb_get_buffer_size();
	int buffer_left;
	

private:
	unsigned char * buffer_ad;
	// unsigned char buffer_ad[100*1024];
	int buffer_id;
	int buffer_size;
	int buffer_head;
	int buffer_tail;
	int readptr;
	
	ak_mutex_t buffer_lock;
	
};

void test_databuf();

#endif // !DATABUFF_H


