#include <iostream>
#include "BaseThread.h"
#include "DataBuff.h"
#include "xmu_vi.h"
extern "C"{
#include "ak_common.h"
}

using namespace std;

int main()
{
    /* init sdk running */
    sdk_run_config config;
    config.mem_trace_flag = SDK_RUN_DEBUG;
    ak_sdk_init( &config );

    cout << "hello world!" << endl;
    test_thread();
    // test_databuf();
    Vi vi;
    vi.start();
    while (1);
    
}
