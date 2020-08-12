#ifndef VDEC_H264_H
#define VDEC_H265_H
#include "xmu_vo.h"
#include "BaseThread.h"
#include "DataBuff.h"

class Vdech264 :public BaseThread
{
public:
	Vdech264(Vo& v);
	~Vdech264();
	void run();
	void start();
	int get_handle_id() { return handle_id; }
	//static ak_mutex_t refresh_flag_lock;
	//static int handle_id[MAX_DE_NUM];
	//static int handle_num = 0;
	//static int refresh_flag = 0;
	//static int refresh_record_flag = 0;
private:
	Vo vo;
	int handle_id;
};


class VdecSend :public BaseThread
{
public:
	VdecSend(Vo& v, int handle_id);
	~VdecSend();
	void run();
	void start();
private:
	DataBuffer dbf;
	Vo vo;
	int handle_id;
};

#endif // !VDEC_H264_H
