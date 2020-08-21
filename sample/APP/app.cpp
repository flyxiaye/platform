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
    test_rtp_send();
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
        adec_send.dbf->rb_write(*ai.dbf, 4096);
    }
}

void test_ai_tcp()
{
    Ai ai;
    MTcpclient client;
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
    Rtp rtp;
    rtp.dbf = vi.dbf;
    vi.start();
    rtp.start();
    while(1);
}