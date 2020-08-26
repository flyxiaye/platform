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
#include "run_sig.h"

extern "C"{
#include "ak_common.h"
#include "ak_drv.h"
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
void test_app(const char *ip, int send_port, int recv_port, int flag);
void test_rtp_ai_ao(int flag);
void test_rtp_vi();
void run_app(const char *ip, int send_port, int recv_port);

using namespace std;


int main(int argc, char **argv)
{
    /* init sdk running */
    sdk_run_config config;
    // config.mem_trace_flag = SDK_RUN_DEBUG;
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    struct key_event key;
    memset(&key, 0, sizeof(key));
    ak_drv_key_open();
    cout << "hello world!" << endl;
    
    char *ip = argv[1];
    int send_port = atoi(argv[2]);
    int recv_port = atoi(argv[3]);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8004);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    struct sockaddr_in clieaddr;
    memset(&clieaddr, 0, sizeof(clieaddr));
    clieaddr.sin_family = AF_INET;
    clieaddr.sin_port = htons(8004);
    clieaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int sock = socket(AF_INET,SOCK_DGRAM,0);
    RunState *rs_send = new RunState(sock, &servaddr, MSG_OUT);
    RunState *rs_recv = new RunState(sock, &clieaddr, MSG_IN);
    int tmp = 1, tmp2 = 0;
    int *run_state = &tmp;
    int *run_state_p = &tmp2;
    rs_send->run_state = run_state;
    rs_recv->run_state = run_state_p;
    rs_send->start();
    rs_recv->start();
    while(1){
        if (*run_state && *run_state_p){
            run_app(ip, send_port, recv_port);
            while(*run_state)
            {
                ak_drv_key_get_event(&key, 50);
                if (key.code == 116 && key.stat == 1)
                {
                    *run_state_p = 0;
                }
                if (*run_state_p == 0)
                {
                    *run_state = 0;
                }
            }
            cout << "end" << endl;
            break;
        }
    }
    while(*run_state || *run_state_p);
    ak_sleep_ms(500);
    delete rs_send, rs_recv;
    

 


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
    // test_app(argv[1], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    // if (!strcmp(argv[1], "server"))
    //     test_rtp_ai_ao(1);
    // else
    //     test_rtp_ai_ao(0);
    // test_rtp_vi();
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
    Rtp rtp("192.168.1.8", 8000);
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
    RtpRecv rtrc(8000);
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
        

        // Vi vi;
        // Vo vo;
        // Vdech264 v1(&vo);
        // VdecSend v2(&vo, v1.get_handle_id());
        // Rtp rtp_vi("192.168.1.9", 8003);
        // vi.rtp = &rtp_vi;
        // RtpRecv rtrc_vi(8002);
        // rtrc_vi.dbf = v2.dbf;
        // vi.start();
        // v1.start();
        // v2.start();
        // rtp_vi.start();
        // rtrc_vi.start();

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
        

        // Vi vi;
        // Vo vo;
        // Vdech264 v1(&vo);
        // VdecSend v2(&vo, v1.get_handle_id());
        // Rtp rtp_vi("192.168.1.10", 8002);
        // vi.rtp = &rtp_vi;
        // RtpRecv rtrc_vi(8003);
        // rtrc_vi.dbf = v2.dbf;
        // vi.start();
        // v1.start();
        // v2.start();
        // rtp_vi.start();
        // rtrc_vi.start();

        rtrc.join();
    }
}

void test_rtp_ai_vi_send()
{
    Ai ai;
    Rtp rtp("192.168.1.8", 8000);
    ai.rtp = &rtp;
    rtp.start();
    ai.start();

    Vi vi;
    Rtp rtp_vi("192.168.1.8", 8001);
    vi.rtp = &rtp_vi;
    vi.start();
    rtp_vi.start();

    rtp_vi.join();
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

    rtrc_vo.join();
}

void test_app(const char *ip, int send_port, int recv_port, int flag)
{
    Ai *ai = new Ai();
    Rtp *rtp = new Rtp(ip, send_port);
    ai->rtp = rtp;

    Vi *vi = new Vi();
    Rtp *rtp_vi = new Rtp(ip, send_port+1);
    vi->rtp = rtp_vi;
    // Vi *vi = NULL;
    // Rtp *rtp_vi = NULL;

    Ao *ao = new Ao();
    Adec *adec = new Adec(ao);
    AdecSend *adec_send = new AdecSend(adec);
    RtpRecv *rtrc = new RtpRecv(recv_port);
    rtrc->dbf_aac = adec_send->dbf;

    Vo *vo = new Vo();
    Vdech264 *v1 = new Vdech264(vo);
    VdecSend *v2 = new VdecSend(vo, v1->get_handle_id());
    RtpRecv *rtrc_vo = new RtpRecv(recv_port+1);
    rtrc_vo->dbf = v2->dbf;
    
    flag &= 0x0f;
    /* 1 ai
       2 vi
       4 ao
       8 vo */
    if (flag == 1){
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        ai->join();
    }

    if (flag == 2){
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        vi->join();
    }

    if (flag == 3)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        vi->join();
    }

    if (flag == 4)
    {
        // delete vi;
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        rtrc->join();
    }
    
    if (flag == 5)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        rtrc->join();
    }

    if (flag == 6)
    {
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
    }

    if (flag == 7)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        rtrc->join();
    }

    if (flag == 8)
    {
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 9)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 10)
    {
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 11)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 12)
    {
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 13)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 14)
    {
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }

    if (flag == 15)
    {
        cout << "ai start" << endl;
        rtp->start();
        ai->start();
        cout << "vi start" << endl;
        rtp_vi->start();
        vi->start();
        cout << "ao start" << endl;
        adec_send->start();
        adec->start();
        rtrc->start();
        cout << "vo start" << endl;
        v1->start();
        v2->start();
        rtrc_vo->start();
        rtrc_vo->join();
    }
    // if (flag & 0x04) {
    //     cout << "ao start" << endl;
    //     adec_send->start();
    //     adec->start();
    //     rtrc->start();
    // }
    
    // if (flag == 8) {
    //     cout << "vo start" << endl;
    //     v1->start();
    //     v2->start();
    //     rtrc_vo->start();
    // }
    
    // rtrc->join();
    // rtrc_vo->join();
    // ai->join();
    // vi->join();
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

void run_app(const char *ip, int send_port, int recv_port)
{
    Ai *ai = new Ai();
    Rtp *rtp = new Rtp(ip, send_port);
    ai->rtp = rtp;

    Vi *vi = new Vi();
    Rtp *rtp_vi = new Rtp(ip, send_port+1);
    vi->rtp = rtp_vi;
    // Vi *vi = NULL;
    // Rtp *rtp_vi = NULL;
    rtp_vi->r = rtp;

    Ao *ao = new Ao();
    Adec *adec = new Adec(ao);
    AdecSend *adec_send = new AdecSend(adec);
    RtpRecv *rtrc = new RtpRecv(recv_port);
    rtrc->dbf_aac = adec_send->dbf;


    Vo *vo = new Vo();
    Vdech264 *v1 = new Vdech264(vo);
    VdecSend *v2 = new VdecSend(vo, v1->get_handle_id());
    RtpRecv *rtrc_vo = new RtpRecv(recv_port+1);
    rtrc_vo->dbf = v2->dbf;
    v1->rtrc_vo = rtrc_vo;
    v1->rtrc_ao = rtrc;

    cout << "ai start" << endl;
    rtp->start();
    ai->start();
    cout << "vi start" << endl;
    rtp_vi->start();
    vi->start();
    cout << "ao start" << endl;
    adec_send->start();
    adec->start();
    rtrc->start();
    cout << "vo start" << endl;
    v1->start();
    v2->start();
    rtrc_vo->start();
}