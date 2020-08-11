#include "DataPull.h"

DataPull::DataPull(int module_id, int bf_size)
{
	dbf = DataBuffer(module_id, bf_size);
}

DataPull::DataPull(int module_id)
{
	dbf = DataBuffer(module_id);
}

DataPull::~DataPull()
{
}