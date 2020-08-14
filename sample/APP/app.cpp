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

using namespace std;

int main(int argc, char **argv)
{
    /* init sdk running */
    sdk_run_config config;
    // config.mem_trace_flag = SDK_RUN_DEBUG;
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    cout << "hello world!" << endl;
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
    test_ai_ao();
}

void test_vi_vo()
{
    Vi vi;
    Vo vo;
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    // v2.vi = &vi;
    // while (1);
    vi.start();
    v1.start();
    v2.start();
    // unsigned char *tmp = (unsigned char *) ak_mem_alloc(MODULE_ID_MEMORY, 10*1024);
    // int read_len;
    // while (1)
    // {
    //     vi.dbf.rb_read(tmp, 1024, &read_len);
    //     v2.dbf.rb_write(tmp, read_len);
    // }
    while(1){
        ak_timeval tim1, tim2;
        ak_get_ostime(&tim1);
        v2.dbf.rb_write(vi.dbf, 60*1024);
        ak_get_ostime(&tim2);
        long tim = ak_diff_ms_time(&tim2, &tim1);
        // ak_print_normal(MODULE_ID_MEMORY, "move mem time %ld\n", tim);
        ak_sleep_ms(10);
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
    // v2.vi = &vi;
    // while (1);
    MTcpServer server;
    MTcpclient client;
    server.dbf = &v2.dbf;
    client.dbf = &vi.dbf;
    server.start();
    client.start();
    vi.start();
    v1.start();
    v2.start();
    while(1);
}

void test_send()
{
    Vi vi;
    MTcpclient client;
    client.dbf = &vi.dbf;
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
    server.dbf = &v2.dbf;
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
    Ao ao;
    Adec adec(&ao);
    AdecSend adec_send(&adec);
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
        adec_send.dbf.rb_write(ai.dbf, 4096);
    }
}