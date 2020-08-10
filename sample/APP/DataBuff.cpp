#include "xmu_common.h"
#include "DataBuff.h"



DataBuffer::DataBuffer(int module_id, int size)
{
	buffer_size = size;
	int ret = ak_mem_rb_init(&buffer_id, module_id, buffer_size);

}

DataBuffer::~DataBuffer()
{
	ak_mem_rb_destroy(buffer_id);
}

int DataBuffer::rb_write(unsigned char* src, int write_len)
{
	return ak_mem_rb_write(buffer_id, src, write_len);
}

int DataBuffer::rb_read(unsigned char* dst, int read_len, int* result)
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
