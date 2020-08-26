#include "rtp_recv.h"
#include <memory>

#define RTP_HEADER_LEN 12
#define NALU_STARTER_LEN 4 // [00 00 00 01] 的开始码
#define NALU_HEADER_LEN 1
#define FU_INDICATOR_LEN 1
#define FU_HEADER_LEN 1

const int MYPORT = 8000;

RtpRecv::RtpRecv()
{
    RtpRecv(MYPORT);
}
RtpRecv::RtpRecv(int port)
{
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ak_print_error_ex(MODULE_ID_THREAD, "socket error\n");
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    printf("监听%d端口\n",MYPORT);
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ak_print_error_ex(MODULE_ID_THREAD, "bind error\n");
}

RtpRecv::~RtpRecv()
{
    // close(sock);
}

void RtpRecv::run()
{
    unsigned char recvbuf[1500] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;
    
    while (1)
    {
        peerlen = sizeof(peeraddr);
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n <= 0)
        {
            ak_print_error_ex(MODULE_ID_THREAD, "recvfrom error");
            continue;
        }
        else if(n > 0)
        {
            rtp_buffer_unpack(recvbuf, n);
        }
    }
}

static void * callback(void * arg)
{
    ((RtpRecv*)arg)->run();
}

void RtpRecv::start()
{
    BaseThread::start(callback);
}


int RtpRecv::rtp_buffer_unpack(unsigned char *write_buf, int write_size)
{
    RTP_FIXED_HEADER *rtp_header = (RTP_FIXED_HEADER *)write_buf;
    timestamps = rtp_header->timestamp;
    // ak_print_normal(2, "timstamp %d, %u\n", rtp_header->payload, rtp_header->timestamp);
    if (rtp_header->payload == H264)
    {
        deal_h264(write_buf, write_size);
    }
    else if (rtp_header->payload == AAC)
    {
        deal_aac(write_buf, write_size);
    }
}

void RtpRecv::deal_h264(unsigned char *write_buf, int write_size)
{
    unsigned char *rtp_payload = write_buf + RTP_HEADER_LEN; // 地址偏移
    uint8_t nalu_type = rtp_payload[0] & 0x1F; // NaluHeader的后5位 或 FuIndicator的后5位 
    if (nalu_type == 0x1C) // 0x1C
    {
        FU_HEADER* fua_head = (FU_HEADER*)(rtp_payload+1); // FuHeader 的前三位
        int header_len = RTP_HEADER_LEN + FU_INDICATOR_LEN + FU_HEADER_LEN;
        unsigned char *nalu_payload = write_buf + header_len; // 地址偏移

        if (fua_head->S == 1) // Fu包为 Nalu的起始位置，需要写入 NaluStarter + NaluHeader + NaluPayload.
        {
            uint8_t nalu_header = (rtp_payload[0] & 0xE0) | (rtp_payload[1] & 0x1F); // FuIndicator 的前3位和 FuHeader的后5位

            dbf->rb_write(&nalu_header, NALU_HEADER_LEN);
            dbf->rb_write(nalu_payload, write_size - header_len);
        }
        else // Fu包为 Nalu的其他位置，只需要写入 NaluPayload.
        {
            dbf->rb_write(nalu_payload, write_size - header_len);
        }
    }
    else // 完整 Nalu 包需要写入 NaluStarter + NaluHeader + NaluPayload.
    {
        dbf->rb_write(rtp_payload, write_size - RTP_HEADER_LEN);
    }
}

void RtpRecv::deal_aac(unsigned char *write_buf, int write_size)
{
    dbf_aac->rb_write(write_buf + RTP_HEADER_LEN, write_size - RTP_HEADER_LEN);
}