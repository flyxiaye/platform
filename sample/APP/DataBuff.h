#ifndef DATABUFF_H
#define DATABUFF_H
extern "C" {
#include "ak_mem.h"
}


class DataBuffer
{
public:
	DataBuffer(int module_id=0, int size = 100 * 1024);
	~DataBuffer();
	int rb_write(unsigned char * src, int write_len);
	int rb_read(unsigned char * dst, int read_len, int* result);
	int rb_get_buffer_id() { return buffer_id; }

private:
	int buffer_id;
	int buffer_size;
};

void test_databuf();

#endif // !DATABUFF_H


