#include <iostream>
#include "BaseThread.h"
#include "DataBuff.h"
#include "xmu_vi.h"
#include "vdec_h264.h"
extern "C"{
#include "ak_common.h"
}
void test_vi_vo();
using namespace std;

int main()
{
    /* init sdk running */
    sdk_run_config config;
    config.mem_trace_flag = SDK_RUN_DEBUG;
    // config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    cout << "hello world!" << endl;
    // test_thread();
    // test_databuf();
    // test_vdech264();
    test_vi_vo();
    // Vi vi;
    // vi.start();
    // while (1);
    
}

void test_vi_vo()
{
    Vi vi;
    Vo vo;
    ak_print_normal(MODULE_ID_VO, "vo ok\n");
    Vdech264 v1(&vo);
    VdecSend v2(&vo, v1.get_handle_id());
    v2.vi = &vi;
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
    while(1);
}
