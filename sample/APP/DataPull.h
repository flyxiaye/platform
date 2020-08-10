#ifndef DATAPULL_H
#define DATAPULL_H
#include "BaseThread.h"
#include "DataBuff.h"

class DataPull
{
public:
	DataPull();
	~DataPull();
	void data_play();
	void data_decode();

private:
	DataBuffer dbf;
};


#endif // !DATAPULL_H
