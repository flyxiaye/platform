#include <exception>
#include <string>
#include <iostream>

#include "xmu_common.h"
#include "DataBuff.h"
extern "C" {
#include "ak_log.h"
#include "ak_common.h"
}
#define MYQUE


DataBuffer::DataBuffer(int module_id, int size)
{
#ifdef MYQUE
	ak_thread_mutex_init(&buffer_lock, NULL);
	buffer_size = size;
	buffer_head = 0;
	buffer_tail = 0;
	buffer_left = buffer_size;
	buffer_ad = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, buffer_size);
#else
	buffer_size = size;
	int ret = ak_mem_rb_init(&buffer_id, module_id, buffer_size);
	if (ret)
	{
		ak_print_error_ex(module_id, "init DataBuffer failed\n");
		throw;
	}
	ret = ak_mem_rb_read_request(buffer_id, &readptr);
#endif
}

DataBuffer::~DataBuffer()
{
#ifdef MYQUE
	ak_mem_free(buffer_ad);
#else
	ak_mem_rb_destroy(buffer_id);
#endif
}

int DataBuffer::rb_write(unsigned char * src, int write_len)
{
#ifdef MYQUE
	ak_thread_mutex_lock(&buffer_lock);
	if (write_len  == 0)
	{
		ak_thread_mutex_unlock(&buffer_lock);
		return AK_SUCCESS;
	}
	if (write_len > buffer_left)
	{
		// ak_print_error_ex(MODULE_ID_MEMORY, "buffer is full\n");
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
	// ak_print_normal(MODULE_ID_MEMORY, "enc buffer left %d\n", buffer_left);
	ak_thread_mutex_unlock(&buffer_lock);
	return AK_SUCCESS;

#else
	int ret = ak_mem_rb_write(buffer_id, src, write_len);
	if (ret)
	{
		ak_print_error_ex(MODULE_ID_MEMORY, "full of repeat buffer\n");
		return FAILED;
	}
	return SUCCESS;
#endif
}

int DataBuffer::rb_read(unsigned char * dst, int read_len, int* result)
{
#ifdef MYQUE
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
	// ak_print_normal(MODULE_ID_MEMORY, "dec buffer left %d\n", buffer_left);
	ak_thread_mutex_unlock(&buffer_lock);
	return AK_SUCCESS;
#else
	int readptr;
	int ret;
	ak_mem_rb_read_request(buffer_id, &readptr);
	// printf("buffer address: %d\n", readptr);
	// if (ret)
	// {
	// 	ak_print_error_ex(MODULE_ID_MEMORY, "pointer request error\n");
	// 	return AK_FAILED;
	// }
	ret = ak_mem_rb_read(buffer_id, readptr, dst, read_len, result);
	// ret = ak_mem_rb_read(buffer_id, readptr, dst, read_len, result);
	if (ret)
	{
		ak_print_error_ex(MODULE_ID_MEMORY, "data read error\n");
		return AK_FAILED;
	}
	ak_mem_rb_read_cancel(buffer_id, readptr);
	// ak_mem_rb_reset(buffer_id);
	return AK_SUCCESS;
#endif
}

int DataBuffer::rb_write(DataBuffer &df_src, int write_len)
{
#ifdef MYQUE
	ak_thread_mutex_lock(&this->buffer_lock);
	if (write_len > buffer_left)
	{
		ak_thread_mutex_unlock(&this->buffer_lock);
		return AK_FAILED;
	}
	int result = 0;
	if (buffer_head < buffer_tail || buffer_head + write_len < buffer_size){
		df_src.rb_read(this->buffer_ad + buffer_head, write_len, &result);
	} else{
		int result2;
		df_src.rb_read(this->buffer_ad + buffer_head, buffer_size - buffer_head, &result2);
		result += result2;
		if (result < write_len){
			df_src.rb_read(this->buffer_ad, write_len - result, &result2);
			result += result2;
		}
	}
	buffer_head += result;
	buffer_head %= buffer_size;
	buffer_left -= result;
	// ak_print_normal(MODULE_ID_MEMORY, "move mem size: %d\n", result);
	ak_thread_mutex_unlock(&this->buffer_lock);
	return AK_SUCCESS;
#else
	
	return AK_SUCCESS;
#endif
}
// int DataBuffer::rb_read(DataBuffer &df_dst);

int DataBuffer::rb_get_buffer_size()
{
#ifdef MYQUE
	return buffer_size - buffer_left;
#else
	int size;
	ak_mem_rb_get_data_size(buffer_id, &size);
	return size;
#endif
}




void test_databuf()
{
	using namespace std;
	DataBuffer d[2];
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
	cout << "buffer size: " << dbf.rb_get_buffer_size() << endl;
	string out = string((char *)dat);
	cout << out << endl << "data size: " << result << endl;
	dbf.rb_read(dat, 20, &result);
	cout << "buffer size: " << dbf.rb_get_buffer_size() << endl;
	out = string((char *)dat);
	cout << out << endl << "data size: " << result << endl;
}