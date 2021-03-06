#ifndef VDEC_H264_H
#define VDEC_H264_H
#include "xmu_vo.h"
#include "BaseThread.h"
#include "DataBuff.h"
#include "xmu_vi.h"
#include "rtp_recv.h"
extern "C"{
#include "ak_common.h"
}

class Vdech264 :public BaseThread
{
public:
	Vdech264(Vo* v);
	~Vdech264();
	void run();
	void start();
	int get_handle_id() { return handle_id; }
	int handle_id;
	RtpRecv *rtrc_vo;
	RtpRecv *rtrc_ao;
private:
	Vo *vo;
	
	// ak_timeval tim1, tim2;
};


class VdecSend :public BaseThread
{
public:
	VdecSend(Vo* v, int handle_id);
	~VdecSend();
	void run();
	void start();
	DataBuffer * dbf;
	Vi *vi;

private:
	Vo* vo;
	int handle_id;
	ak_timeval tim1, tim2;
};

void test_vdech264();

#endif // !VDEC_H264_H
