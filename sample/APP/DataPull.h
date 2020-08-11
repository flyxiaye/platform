#ifndef DATAPULL_H
#define DATAPULL_H
#include "BaseThread.h"
#include "DataBuff.h"

class DataPull :public BaseThread
{
public:
	//DataPull();
	DataPull(int module_id, int bf_size);
	DataPull(int module_id);
	~DataPull();
	virtual void data_play() = 0;
	virtual void data_decode() = 0;

private:
	DataBuffer dbf;
};


#endif // !DATAPULL_H
