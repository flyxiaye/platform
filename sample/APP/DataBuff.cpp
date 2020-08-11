#include <exception>
#include <string>
#include <iostream>

#include "xmu_common.h"
#include "DataBuff.h"
extern "C" {
#include "ak_log.h"
#include "ak_common.h"
}



DataBuffer::DataBuffer(int module_id, int size)
{
	buffer_size = size;
	int ret = ak_mem_rb_init(&buffer_id, module_id, buffer_size);
	if (ret)
	{
		ak_print_error_ex(module_id, "init DataBuffer failed\n");
		throw;
	}
}

DataBuffer::~DataBuffer()
{
	ak_mem_rb_destroy(buffer_id);
}

int DataBuffer::rb_write(unsigned char * src, int write_len)
{
	int ret = ak_mem_rb_write(buffer_id, src, write_len);
	if (ret)
	{
		ak_print_error_ex(MODULE_ID_MEMORY, "full of repeat buffer\n");
		return FAILED;
	}
	return SUCCESS;
}

int DataBuffer::rb_read(unsigned char * dst, int read_len, int* result)
{
	int readptr;
	int ret;
	ret = ak_mem_rb_read_request(buffer_id, &readptr);
	if (ret)
		return FAILED;
	ret = ak_mem_rb_read(buffer_id, readptr, dst, read_len, result);
	if (ret)
		return FAILED;
	return SUCCESS;
}


void test_databuf()
{
	using namespace std;
	DataBuffer dbf(MODULE_ID_MEMORY);
	string s = "hello!";
	dbf.rb_write((unsigned char *)s.c_str(), s.length()+1);
	unsigned char* dat = new unsigned char[64];
	int result = 0;
	dbf.rb_read(dat, 64, &result);
	string out = string((char *)dat);
	cout << out << endl << "data size: " << result << endl;
}