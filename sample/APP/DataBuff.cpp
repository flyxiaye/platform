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
	ak_thread_mutex_init(&buffer_lock, NULL);
	buffer_size = size;
	buffer_head = 0;
	buffer_tail = 0;
	buffer_left = buffer_size;
	buffer_ad = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, buffer_size);
	
	// int ret = ak_mem_rb_init(&buffer_id, module_id, buffer_size);
	// if (ret)
	// {
	// 	ak_print_error_ex(module_id, "init DataBuffer failed\n");
	// 	throw;
	// }
}

DataBuffer::~DataBuffer()
{
	ak_mem_free(buffer_ad);
	// ak_mem_rb_destroy(buffer_id);
}

int DataBuffer::rb_write(unsigned char * src, int write_len)
{
	ak_thread_mutex_lock(&buffer_lock);
	if (write_len  == 0)
	{
		ak_thread_mutex_unlock(&buffer_lock);
		return AK_SUCCESS;
	}
	if (write_len > buffer_left)
	{
		ak_print_error_ex(MODULE_ID_MEMORY, "buffer is full\n");
		ak_thread_mutex_unlock(&buffer_lock);
		return AK_FAILED;
	}
	for (int i = 0; i < write_len; i++)
	{
		*(buffer_ad + buffer_head) = *(src + i);
		buffer_head ++;
		buffer_head %= buffer_size;
	}
	buffer_left -= write_len;
	ak_print_normal(MODULE_ID_MEMORY, "buffer left %d\n", buffer_left);
	ak_thread_mutex_unlock(&buffer_lock);
	return AK_SUCCESS;
	// int ret = ak_mem_rb_write(buffer_id, src, write_len);
	// if (ret)
	// {
	// 	ak_print_error_ex(MODULE_ID_MEMORY, "full of repeat buffer\n");
	// 	return FAILED;
	// }
	// return SUCCESS;
}

int DataBuffer::rb_read(unsigned char * dst, int read_len, int* result)
{
	ak_thread_mutex_lock(&buffer_lock);
	if (read_len > buffer_size - buffer_left)
	{
		*result = buffer_size - buffer_left;
	}
	else
	{
		*result = read_len;
	}
	for (int i = 0; i < *result; i++)
	{
		*(dst + i) = *(buffer_ad + buffer_tail);
		buffer_tail ++;
		buffer_tail %= buffer_size;
	}
	buffer_left += *result;
	ak_print_normal(MODULE_ID_MEMORY, "buffer read %d\n", *result);
	ak_thread_mutex_unlock(&buffer_lock);
	return AK_SUCCESS;
	
	// int readptr;
	// int ret;
	// ret = ak_mem_rb_read_request(buffer_id, &readptr);
	// if (ret)
	// {
	// 	ak_print_error_ex(MODULE_ID_MEMORY, "pointer request error\n");
	// 	return FAILED;
	// }
	// ret = ak_mem_rb_read(buffer_id, readptr + buffer_p, dst, read_len, result);
	// if (ret)
	// {
	// 	ak_print_error_ex(MODULE_ID_MEMORY, "data read error\n");
	// 	return FAILED;
	// }
	// buffer_p += *result;
	// buffer_p %= buffer_size;
	// return SUCCESS;
}

int DataBuffer::rb_get_buffer_size()
{
	// int size;
	// ak_mem_rb_get_data_size(buffer_id, &size);
	// return size;
	return buffer_size;
}




void test_databuf()
{
	using namespace std;
	DataBuffer dbf(MODULE_ID_MEMORY);
	string s = "hello!";
	cout << "length s:" << s.length() << endl;
	dbf.rb_write((unsigned char *)s.c_str(), s.length());
	dbf.rb_write((unsigned char *)s.c_str(), s.length());
	dbf.rb_write((unsigned char *)s.c_str(), s.length());
	cout << "buffer size: " << dbf.rb_get_buffer_size() << endl;
	unsigned char* dat = new unsigned char[64];
	int result = 0;
	dbf.rb_read(dat, 5, &result);
	string out = string((char *)dat);
	cout << out << endl << "data size: " << result << endl;
	dbf.rb_read(dat, 20, &result);
	out = string((char *)dat);
	cout << out << endl << "data size: " << result << endl;
}