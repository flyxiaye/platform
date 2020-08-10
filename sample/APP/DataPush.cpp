#include "DataPush.h"

DataPush::DataPush(int module_id, int bf_size)
{
	dbf = DataBuffer(module_id, bf_size);
}

DataPush::~DataPush()
{
}