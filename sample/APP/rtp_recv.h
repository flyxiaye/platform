#ifndef RTP_RECV_H
#define RTP_RECV_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "rtp.h"
#include "BaseThread.h"
#include "DataBuff.h"

extern "C" {
#include "ak_mem.h"
#include "ak_common.h"
}

class RtpRecv :public BaseThread
{
public:
    RtpRecv();
    ~RtpRecv();
    void run();
    void start();
    DataBuffer *dbf;

private:
    int sock;
    int rtp_buffer_unpack(unsigned char *write_buf, int write_size);
};

#endif