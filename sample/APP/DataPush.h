#ifndef DATAPUSH_H
#define DATAPUSH_H
#include "BaseThread.h"
#include "DataBuff.h"

class DataPush :BaseThread
{
public:
	DataPush(int module_id, int bf_size);
	~DataPush();
	virtual void data_collect() = 0;
	virtual void data_encode() = 0;

private:
	DataBuffer dbf;
};



#endif // !DATAPUSH_H
