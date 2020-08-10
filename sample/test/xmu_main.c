#include "xmu_main.h"
#include "xmu_vi.h"
#include "xmu_venc.h"
#include "xmu_vo.h"
#include "xmu_vdec.h"
#include "xmu_common.h"

#include "ak_vi.h"

void send_data(void)
{
    vi_set_param();
    venc_set_param();
    vi_init();
    venc_init();
    venc_start();
    struct video_input_frame frame;
    enc_pair_set_source(&frame);
	venc_start(); 	//线程启动
    while(1)
    {
        int ret = vi_get_one_frame(&frame, sizeof(struct video_input_frame));
        if (ret == SUCCESS)
        {
            // vo_put_one_frame(frame.vi_frame.data);
			venc_thread_sem_post(); 	//通知编码线程启动
			vi_release_one_frame(&frame);
        }
    }
}

void receive_data(void)
{
    vo_set_param();
    vdec_set_param();
    vo_init();
    vdec_init();
    vdec_start();
    
}