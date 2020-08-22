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
    // int sock;
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

    input_buf = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, 1500);

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
        ak_print_normal(0, "%d\n", 0);
        peerlen = sizeof(peeraddr);
        // memset(recvbuf, 0, sizeof(recvbuf));
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n <= 0)
        {
            ak_print_error_ex(MODULE_ID_THREAD, "recvfrom error");
            continue;
        }
        else if(n > 0)
        {
            ak_print_normal(0, "buf size %d\n", n);
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
    // ak_print_normal(MODULE_ID_VDEC, "timestamp %lu\n", rtp_header->timestamp); 
    // ak_print_normal(MODULE_ID_VDEC, "payload %d\n", rtp_header->payload); 
    ak_print_normal(0, "payload %d\n", rtp_header->payload);
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
    // ak_print_normal(MODULE_ID_VDEC, "timestamp %lu\n", rtp_header->timestamp); 
    if (nalu_type == 0x1C) // 0x1C
    {
        FU_HEADER* fua_head = (FU_HEADER*)(rtp_payload+1); // FuHeader 的前三位
        int header_len = RTP_HEADER_LEN + FU_INDICATOR_LEN + FU_HEADER_LEN;
        unsigned char *nalu_payload = write_buf + header_len; // 地址偏移

        if (fua_head->S == 1) // Fu包为 Nalu的起始位置，需要写入 NaluStarter + NaluHeader + NaluPayload.
        {
            uint8_t nalu_header = (rtp_payload[0] & 0xE0) | (rtp_payload[1] & 0x1F); // FuIndicator 的前3位和 FuHeader的后5位

            // int input_size = NALU_HEADER_LEN + (write_size - header_len);
            // unsigned char *input_buf = (unsigned char*)ak_mem_alloc(MODULE_ID_MEMORY, input_size);
            // memset(input_buf, 0, input_size);
            // input_buf[NALU_STARTER_LEN - 1] = 1; // NaluStarter - [00, 00, 00, 01]
            dbf->rb_write(&nalu_header, NALU_HEADER_LEN);
            dbf->rb_write(nalu_payload, write_size - header_len);
            // memcpy(input_buf, &nalu_header, NALU_HEADER_LEN); // NaluHeader
            // memcpy(input_buf + NALU_HEADER_LEN, nalu_payload, write_size - header_len); // NaluPayload
            // dbf->rb_write(input_buf, input_size);
            // ak_mem_free(input_buf);
            // return h264_buffer_input(input_buf, input_size);
        }
        else // Fu包为 Nalu的其他位置，只需要写入 NaluPayload.
        {
            // int input_size = write_size - header_len;
            // unsigned char *input_buf = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, input_size);
            // memset(input_buf, 0, input_size);
            dbf->rb_write(nalu_payload, write_size - header_len);
            // memcpy(input_buf, nalu_payload, input_size); // NaluPayload
            // dbf->rb_write(input_buf, input_size);
            // ak_mem_free(input_buf);
        }
    }
    else // 完整 Nalu 包需要写入 NaluStarter + NaluHeader + NaluPayload.
    {
        // int input_size = write_size - RTP_HEADER_LEN;
        // unsigned char *input_buf = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, input_size);
        // memset(input_buf, 0, input_size);
        // input_buf[NALU_STARTER_LEN - 1] = 1; // NaluStarter - [00, 00, 00, 01]
        dbf->rb_write(rtp_payload, write_size - RTP_HEADER_LEN);
        // memcpy(input_buf, rtp_payload, write_size - RTP_HEADER_LEN); // RtpPayload = NaluHeader + NaluPayload.
        // dbf->rb_write(input_buf, input_size);
        // ak_mem_free(input_buf);
        // return h264_buffer_input(input_buf, input_size);
    }
}

void RtpRecv::deal_aac(unsigned char *write_buf, int write_size)
{
    // return;
    // unsigned char *rtp_payload = write_buf + RTP_HEADER_LEN; // 地址偏移
    // // uint8_t nalu_type = rtp_payload[0] & 0x1F; // NaluHeader的后5位 或 FuIndicator的后5位 
    // // ak_print_normal(MODULE_ID_VDEC, "timestamp %lu\n", rtp_header->timestamp); 

    // int input_size = write_size - RTP_HEADER_LEN;
    // unsigned char *input_buf = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, input_size);
    // memset(input_buf, 0, input_size);
    // input_buf[NALU_STARTER_LEN - 1] = 1; // NaluStarter - [00, 00, 00, 01]
    // memcpy(input_buf, rtp_payload, write_size - RTP_HEADER_LEN); // RtpPayload = NaluHeader + NaluPayload.
    // dbf_aac->rb_write(input_buf, input_size);
    // ak_mem_free(input_buf);
    dbf_aac->rb_write(write_buf + RTP_HEADER_LEN, write_size - RTP_HEADER_LEN);
}