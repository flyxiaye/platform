#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "rtp.h"
 
// const char* DEST_IP = "127.0.0.1";
const char* DEST_IP = "192.168.1.9";
const int DEST_PORT = 8000;

//为NALU_t结构体分配内存空间
Rtp::Rtp()
{
	Rtp(DEST_IP, DEST_PORT);
}

Rtp::Rtp(int port)
{
	Rtp(DEST_IP, port);
}

Rtp::Rtp(const char * ip, int port)
{
	this->ip = (char *)ip;
	this->port = port;

	ak_thread_sem_init(&sem2, 0);

	wait_sem = 0;
	seq_num =0;
	bytes=0;
	len =sizeof(server);
	framerate=25;
	ts_current=0;
	timestamp_increse=(unsigned long)(90000.0 / framerate); //+0.5);  //时间戳，H264的视频设置成90000
	payload = 0;

	server.sin_family=AF_INET;
	server.sin_port=htons(this->port);          
	server.sin_addr.s_addr=inet_addr(this->ip); 
	socket1=socket(AF_INET,SOCK_DGRAM,0);

}

Rtp::~Rtp()
{
}

 
void Rtp::run()
{
	ak_print_normal(MODULE_ID_THREAD, "rtp thread start!\n");
	while(1) 
	{
		// int l = GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		// if (l != -1) printf("len of nalu %d\n", l);
		ak_thread_sem_wait(&sem);
		// dump(n);//输出NALU长度和TYPE
		if (payload == H264)
		{
			deal_h264();
		}
		else if (payload == AAC)
		{
			deal_aac();
		}
		wait_sem = 0;
		ak_thread_sem_post(&sem2);
	}
}
 

static void *callback(void *arg)
{
	((Rtp*)arg)->run();
}

void Rtp::start()
{
	BaseThread::start(callback);
}

void Rtp::send(unsigned char * send_data, int stream_len, int payload)
{
	this->stream_buf = send_data;
	this->stream_len = stream_len;
	this->payload = payload;
	ak_thread_sem_post(&sem);

	ak_thread_sem_wait(&sem2);
}


void Rtp::deal_h264()
{
	//（1）一个NALU就是一个RTP包的情况： RTP_FIXED_HEADER（12字节）  + NALU_HEADER（1字节） + EBPS
	//（2）一个NALU分成多个RTP包的情况： RTP_FIXED_HEADER （12字节） + FU_INDICATOR （1字节）+  FU_HEADER（1字节） + EBPS(1400字节)
	//rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。
	rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
	//设置RTP HEADER，
	rtp_hdr->payload     = H264;  //负载类型号，
	rtp_hdr->version     = 2;  //版本号，此版本固定为2
	rtp_hdr->marker    = 0;   //标志位，由具体协议规定其值。
	rtp_hdr->ssrc        = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一
	//	当一个NALU小于1400字节的时候，采用一个单RTP包发送
	if(stream_len <= 1400)
	{	
		//设置rtp M 位；
		rtp_hdr->marker = 1;
		rtp_hdr->seq_no  = htons(seq_num ++); //序列号，每发送一个RTP包增1，htons，将主机字节序转成网络字节序。
		//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
		nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
		nalu_hdr->F = stream_buf[0] & 0x80;
		nalu_hdr->NRI= (stream_buf[0] & 0x60) >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
		nalu_hdr->TYPE= (stream_buf[0]) & 0x1f;

		nalu_payload=&sendbuf[13];//同理将sendbuf[13]赋给nalu_payload
		memcpy(nalu_payload,stream_buf+1,stream_len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

		ts_current = ts_current + timestamp_increse;
		
		// rtp_hdr->timestamp=htonl(ts_current);
		rtp_hdr->timestamp=(ts_current);
		// ak_print_normal(MODULE_ID_VENC, "timestamp %lu\n", rtp_hdr->timestamp); 
		// ak_print_normal(MODULE_ID_VENC, "timestampinc %lu\n", timestamp_increse); 
		bytes=stream_len + 12 ;	//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
		sendto(socket1, sendbuf, bytes, 0, (struct sockaddr*)&server, sizeof(server));//发送rtp包
		// ak_print_normal(MODULE_ID_THREAD, "send success\n");
		//	Sleep(100);
		wait_sem = 0;
	}

	else if(stream_len > 1400)  //这里就要分成多个RTP包发送了。
	{
		//得到该nalu需要用多少长度为1400字节的RTP包来发送
		int k = 0, last = 0;
		k = stream_len / 1400;//需要k个1400字节的RTP包，这里为什么不加1呢？因为是从0开始计数的。
		last = stream_len % 1400;//最后一个RTP包的需要装载的字节数
		int t = 0;//用于指示当前发送的是第几个分片RTP包
		ts_current = ts_current + timestamp_increse;
		rtp_hdr->timestamp = (ts_current);
		// rtp_hdr->timestamp = htonl(ts_current);
		// ak_print_normal(MODULE_ID_VENC, "timestamp %lu\n", rtp_hdr->timestamp); 
		// ak_print_normal(MODULE_ID_VENC, "timestampinc %lu\n", timestamp_increse); 
		while(t <= k)
		{
			rtp_hdr->seq_no = htons(seq_num++); //序列号，每发送一个RTP包增1
			if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位,t = 0时进入此逻辑。
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;  //最后一个NALU时，该值设置成1，其他都设置成0。
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = stream_buf[0] & 0x80;
				fu_ind->NRI = (stream_buf[0] & 0x60) >> 5;
				fu_ind->TYPE = 28;  //FU-A类型。

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr =(FU_HEADER*)&sendbuf[13];
				fu_hdr->E = 0;
				fu_hdr->R = 0;
				fu_hdr->S = 1;
				fu_hdr->TYPE = (stream_buf[0]) & 0x1f;

				nalu_payload = &sendbuf[14];//同理将sendbuf[14]赋给nalu_payload
				memcpy(nalu_payload,stream_buf+1,1400);//去掉NALU头，每次拷贝1400个字节。

				bytes = 1400 + 14;//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度                                                            14字节
				sendto(socket1, sendbuf, bytes, 0, (struct sockaddr*)&server, sizeof(server));//发送rtp包
				t++;

			}
			//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
			else if(k == t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当 l> 1386时）。
			{

				//设置rtp M 位；当前传输的是最后一个分片时该位置1
				rtp_hdr->marker=1;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F=stream_buf[0] & 0x80;
				fu_ind->NRI=(stream_buf[0] & 0x60) >> 5;
				fu_ind->TYPE=28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (FU_HEADER*)&sendbuf[13];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->TYPE = (stream_buf[0]) & 0x1f;
				fu_hdr->E = 1;

				nalu_payload = &sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload,stream_buf + t*1400 + 1,last-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
				bytes = last - 1 + 14;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
				sendto(socket1, sendbuf, bytes, 0, (struct sockaddr*)&server, sizeof(server));//发送rtp包
				t++;
				//Sleep(100);
			}
			//既不是第一个分片，也不是最后一个分片的处理。
			else if(t < k && 0 != t)
			{
				//设置rtp M 位；
				rtp_hdr->marker = 0;
				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
				fu_ind = (FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
				fu_ind->F = stream_buf[0] & 0x80;
				fu_ind->NRI = (stream_buf[0] & 0x60) >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr =(FU_HEADER*)&sendbuf[13];

				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->E = 0;
				fu_hdr->TYPE = (stream_buf[0]) & 0x1f;

				nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
				memcpy(nalu_payload, stream_buf + t * 1400 + 1,1400);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
				bytes=1400 + 14;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
				sendto(socket1, sendbuf, bytes, 0, (struct sockaddr*)&server, sizeof(server));//发送rtp包
				t++;
			}
		}
	}
}

void Rtp::deal_aac()
{
	rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0]; 
	//设置RTP HEADER，
	rtp_hdr->payload     = AAC;  //负载类型号，
	rtp_hdr->version     = 2;  //版本号，此版本固定为2
	rtp_hdr->marker    = 0;   //标志位，由具体协议规定其值。
	rtp_hdr->ssrc        = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一
	rtp_hdr->marker = 1;

	rtp_hdr->seq_no  = htons(seq_num ++); //序列号，每发送一个RTP包增1，htons，将主机字节序转成网络字节序。
	//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
	// nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
	// nalu_hdr->F = stream_buf[0] & 0x80;
	// nalu_hdr->NRI= (stream_buf[0] & 0x60) >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
	// nalu_hdr->TYPE= (stream_buf[0]) & 0x1f;

	nalu_payload=&sendbuf[12];//同理将sendbuf[13]赋给nalu_payload
	memcpy(nalu_payload,stream_buf, stream_len);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

	ts_current = ts_current + timestamp_increse;
	
	// rtp_hdr->timestamp=htonl(ts_current);
	rtp_hdr->timestamp=(ts_current);
	// ak_print_normal(MODULE_ID_VENC, "timestamp %lu\n", rtp_hdr->timestamp); 
	// ak_print_normal(MODULE_ID_VENC, "timestampinc %lu\n", timestamp_increse); 
	bytes=stream_len + 12 ;	//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
	sendto(socket1, sendbuf, bytes, 0, (struct sockaddr*)&server, sizeof(server));//发送rtp包
}