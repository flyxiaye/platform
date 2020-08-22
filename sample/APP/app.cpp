#include <iostream>
#include <string.h>
#include "BaseThread.h"
#include "DataBuff.h"
#include "xmu_vi.h"
#include "vdec_h264.h"
#include "MTcpclient.h"
#include "MTcpServer.h"
#include "xmu_ai.h"
#include "xmu_ao.h"
#include "adec.h"
#include "rtp.h"
#include "rtp_recv.h"

extern "C"{
#include "ak_common.h"
}
void test_vi_vo();
void test_tcp();
void test_tcp_vivo();
void test_send();
void test_recv();
void test_aenc();
void test_adec();
void test_ai_ao();
void test_ai_tcp();
void test_ao_tcp();
void test_rtp_send();
void test_rtp_recv();
void test_rtp_send_recv();
void test_rtp_ai_send();
void test_rtp_ao_recv();
void test_rtp_ai_ao();
void test_rtp_ai_vi_send();
void test_rtp_ao_ao_recv();
void test_app(const char *ip, int send_port, int recv_port);
void test_rtp_ai_ao(int flag);
void test_rtp_vi();

using namespace std;

int main(int argc, char **argv)
{
    /* init sdk running */
    sdk_run_config config;
    // config.mem_trace_flag = SDK_RUN_DEBUG;
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    cout << "hello world!" << endl;
    // MTcpServer s;
    // test_thread();
    // test_databuf();
    // test_vdech264();
    // test_vi_vo();
    // test_tcp();
    // test_tcp_vivo();
    // if (!strcmp(argv[1], "server"))
    //     test_recv();
    // else if (!strcmp(argv[1], "client"))
    //     test_send();
    // Vi vi;
    // vi.start();
    // while (1);
    // test_aenc();
    // test_adec();
    // test_ai_ao();
    // if (!strcmp(argv[1], "server"))
    //     test_ao_tcp();
    // else if (!strcmp(argv[1], "client"))
    //     test_ai_tcp();
    // test_rtp_send();
    // test_rtp_send_recv();
    // if (!strcmp(argv[1], "server"))
    //     test_rtp_recv();
    // else if (!strcmp(argv[1], "client"))
    //     test_rtp_send();
    // if (!strcmp(argv[1], "server"))
    //     test_rtp_ao_recv();
    // else if (!strcmp(argv[1], "client"))
    //     test_rtp_ai_send();
    // test_rtp_ai_ao();
    // if (!strcmp(argv[1], "server"))
    //     test_rtp_ao_ao_recv();
    // else if (!strcmp(argv[1], "client"))
    //     test_rtp_ai_vi_send();
    // test_app(argv[1], atoi(argv[2]), atoi(argv[3]));
    // if (!strcmp(argv[1], "server"))
    //     test_rtp_ai_ao(1);
    // else
    //     test_rtp_ai_ao(0);
    test_rtp_vi();
}

void test_vi_vo()
{
    Vi vi;
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    // MTcpServer s;
    vi.start();
    v1.start();
    v2.start();
    while(1){
        v2.dbf->rb_write(*vi.dbf, 1024);
    }
    while(1);
}

void test_tcp(void)
{
    MTcpServer server;
    MTcpclient client;
    server.start();
    client.start();
    while(1);
}

void test_tcp_vivo()
{
    Vi vi;
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    MTcpServer server;
    server.start();
    MTcpclient client;
    server.dbf = v2.dbf;
    client.start();
    vi.start();
    v1.start();
    v2.start();
    while(1);
}

void test_send()
{
    Vi vi;
    MTcpclient client("192.168.1.9");
    client.dbf = vi.dbf;
    client.start();
    vi.start();
    while(1);
}

void test_recv()
{
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    MTcpServer server;
    server.dbf = v2.dbf;
    server.start();
    v1.start();
    v2.start();
    while(1);
}

void test_aenc()
{
    Ai ai;
    ai.start();
    while(1);
}

void test_adec()
{
    char *filename = (char*)"/mnt/test.mp3";
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("open file failed\n");
        return;
    }
    // fseek(fp, 0, SEEK_END);
    // int len = ftell(fp);
    // rewind(fp);
    unsigned char *buff = (unsigned char *)ak_mem_alloc(MODULE_ID_MEMORY, 200*1024);
    fread(buff, 200*1024, 1, fp);
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    adec_send.dbf->rb_write(buff, 200*1024);
    ak_print_normal(MODULE_ID_ADEC, "buffer size %d\n", adec_send.dbf->rb_get_buffer_size());
    adec.start();
    adec_send.start();
    while(1);
}

void test_ai_ao()
{
    Ai ai;
    
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    ai.start();
    adec.start();
    adec_send.start();
    while(1)
    {
        adec_send.dbf->rb_write(*ai.dbf, 1024);
    }
}

void test_ai_tcp()
{
    Ai ai;
    MTcpclient client("192.168.1.9");
    client.dbf = ai.dbf;
    client.start();
    ai.start();
    while(1);
}

void test_ao_tcp()
{
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    MTcpServer server;
    server.dbf = adec_send.dbf;
    server.start();
    adec_send.start();
    adec.start();
    while(1);
}

void test_rtp_send()
{
    Vi vi;
    Rtp rtp("192.168.1.9", 8000);
    // rtp.dbf = vi.dbf;
    vi.rtp = &rtp;
    vi.start();
    rtp.start();
    while(1);
}

void test_rtp_recv()
{
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    RtpRecv rtrc(8000);
    rtrc.dbf = v2.dbf;
    v1.start();
    v2.start();
    rtrc.start();
    while(1);
}

void test_rtp_send_recv()
{
    Vi vi;
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    Rtp rtp("127.0.0.1", 8000);
    vi.rtp = &rtp;
    RtpRecv rtrc(8000);
    rtrc.dbf = v2.dbf;
    vi.start();
    v1.start();
    v2.start();
    rtp.start();
    rtrc.start();
    while(1);
}

void test_rtp_ai_send()
{
    Ai ai;
    // MTcpclient client("192.168.1.9");
    // client.dbf = ai.dbf;
    // client.start();
    Rtp rtp;
    ai.rtp = &rtp;
    rtp.start();
    ai.start();
    while(1);
}

void test_rtp_ao_recv()
{
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    RtpRecv rtrc;
    rtrc.dbf_aac = adec_send.dbf;
    adec_send.start();
    adec.start();
    rtrc.start();
    while(1);
}

void test_rtp_ai_ao(int flag)
{
    if (flag == 1)
    {
        Ai ai;
        Rtp rtp("192.168.1.9", 8000);
        ai.rtp = &rtp;
        rtp.start();
        ai.start();

        Ao ao;
        Adec adec(&ao);
        AdecSend adec_send(&adec);
        RtpRecv rtrc(8001);
        rtrc.dbf_aac = adec_send.dbf;
        adec_send.start();
        adec.start();
        rtrc.start();
        

        Vi vi;
        Vo vo;
        Vdech264 v1(&vo);
        VdecSend v2(&vo, v1.get_handle_id());
        Rtp rtp_vi("192.168.1.9", 8003);
        vi.rtp = &rtp_vi;
        RtpRecv rtrc_vi(8002);
        rtrc_vi.dbf = v2.dbf;
        vi.start();
        v1.start();
        v2.start();
        rtp_vi.start();
        rtrc_vi.start();

        rtrc.join();
    }
    else
    {
        Ai ai;
        Rtp rtp("192.168.1.10", 8001);
        ai.rtp = &rtp;
        rtp.start();
        ai.start();

        Ao ao;
        Adec adec(&ao);
        AdecSend adec_send(&adec);
        RtpRecv rtrc(8000);
        rtrc.dbf_aac = adec_send.dbf;
        adec_send.start();
        adec.start();
        rtrc.start();
        

        Vi vi;
        Vo vo;
        Vdech264 v1(&vo);
        VdecSend v2(&vo, v1.get_handle_id());
        Rtp rtp_vi("192.168.1.10", 8002);
        vi.rtp = &rtp_vi;
        RtpRecv rtrc_vi(8003);
        rtrc_vi.dbf = v2.dbf;
        vi.start();
        v1.start();
        v2.start();
        rtp_vi.start();
        rtrc_vi.start();

        rtrc.join();
    }
}

void test_rtp_ai_vi_send()
{
    Ai ai;
    Rtp rtp("192.168.1.9", 8000);
    ai.rtp = &rtp;
    rtp.start();
    ai.start();

    Vi vi;
    Rtp rtp_vi("192.168.1.9", 8001);
    vi.rtp = &rtp_vi;
    vi.start();
    rtp_vi.start();

    while(1);
}

void test_rtp_ao_ao_recv()
{
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    RtpRecv rtrc(8000);
    rtrc.dbf_aac = adec_send.dbf;
    adec_send.start();
    adec.start();
    rtrc.start();

    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    RtpRecv rtrc_vo(8001);
    rtrc_vo.dbf = v2.dbf;
    v1.start();
    v2.start();
    rtrc_vo.start();

    while(1);
}

void test_app(const char *ip, int send_port, int recv_port)
{
    Ai ai;
    Rtp rtp(ip, send_port);
    ai.rtp = &rtp;

    Vi vi;
    Rtp rtp_vi(ip, send_port+1);
    vi.rtp = &rtp_vi;

    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    RtpRecv rtrc(recv_port);
    rtrc.dbf_aac = adec_send.dbf;


    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    RtpRecv rtrc_vo(recv_port+1);
    rtrc_vo.dbf = v2.dbf;

    // rtp.start();
    ai.start();

    rtp_vi.start();
    vi.start();
    

    adec_send.start();
    adec.start();
    // rtrc.start();

    v1.start();
    v2.start();
    rtrc_vo.start();

    rtrc_vo.join();
    // while(1);
}

void test_rtp_vi()
{
    Ai ai;
    Rtp rtp("127.0.0.1", 8000);
    ai.rtp = &rtp;

    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
    RtpRecv rtrc(8000);
    rtrc.dbf_aac = adec_send.dbf;

    Vi vi;
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    Rtp rtp_vi("127.0.0.1", 8002);
    vi.rtp = &rtp_vi;
    RtpRecv rtrc_vi(8002);
    rtrc_vi.dbf = v2.dbf;
    vi.rtp = &rtp_vi;
    rtrc_vi.dbf = v2.dbf;

    rtp.start();
    rtp_vi.start();
    ai.start();
    adec_send.start();
    adec.start();

    vi.start();
    v1.start();
    v2.start();
    rtrc.start();
    
    rtrc_vi.start();

    rtrc.join();

}