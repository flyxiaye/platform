#ifndef DATAPUSH_H
#define DATAPUSH_H
#include "BaseThread.h"
#include "DataBuff.h"

class DataPush :BaseThread
{
public:
	DataPush(int module_id, int bf_size);
	~DataPush();
	void data_collect();
	void data_encode();

private:
	DataBuffer dbf;
};



#endif // !DATAPUSH_H
