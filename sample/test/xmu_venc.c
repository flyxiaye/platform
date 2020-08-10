#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"

#include "xmu_common.h"
#include "xmu_venc.h"

#ifdef AK_RTOS
#include "rtthread.h"
#define THREAD_PRIO 90
#else
#define THREAD_PRIO -1
#endif

#define RECORD_READ_LEN      4096*10 /* read video file buffer length */
#define DE_VIDEO_SIZE_MAX    6

/* mipi screen res */
#define MIPI_SC_WIDTH 1280
#define MIPI_SC_HEIGHT 800

/* RGB screen res */
#define RGB_SC_WIDTH  1024
#define RGB_SC_HEIGHT  600

#define MAX_ENC_NUM   4



/* resolation define */
struct resolution_t {
	unsigned int width;
	unsigned int height;
	unsigned char str[20];
};

static char *pc_prog_name = NULL;                      //demo名称
static char *file  = NULL;                             //file name
static char *type         = NULL;                      //get the type input
// static char *save_path    = "/mnt/video_encode";
static int main_res       = 4;
static int sub_res        = 1;
static char *cfg = "/etc/jffs2/isp_pr2000_dvp.conf";
ak_mutex_t refresh_flag_lock;
static int chn_index = 0;
static ak_pthread_t venc_stream_th;
/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {960,  1080,   "DE_VIDEO_SIZE_960"},
    {1920,	1080,  "DE_VIDEO_SIZE_1080P"},
    {2560,  1920,  "DE_VIDEO_SIZE_1920P"}
};


//=============自己定义的变量============//
static ak_sem_t enc_sem;  //编码线程信号量
static struct venc_pair
{
    int venc_handle;
    struct video_input_frame *frame;
    struct video_stream *stream;
    int en_type;
}enc_pair;      //线程传递参数
//=============自己定义的变量end============//

void *video_encode_from_vi_th(void *arg)
{
    ak_thread_set_name("venc_th");
    struct venc_pair *venc_th = (struct venc_pair *)arg; //venc handle
    struct video_input_frame *frame = enc_pair.frame;
    enc_pair.stream = ak_mem_alloc(MODULE_ID_VENC, sizeof(struct video_stream));
    struct video_stream *stream = enc_pair.stream;
    ak_print_normal(MODULE_ID_VI, "capture start\n");

    while(1)
    {
        // ak_print_normal(MODULE_ID_VENC, "behind enc\n");
        ak_thread_sem_wait(&enc_sem);  //线程等待信号量
        // if (-1 == ak_thread_sem_trywait(&enc_sem))
        //     continue;
        ak_print_normal(MODULE_ID_VENC, "after enc\n");
        int ret = ak_venc_encode_frame(venc_th->venc_handle, frame->vi_frame.data, frame->vi_frame.len, frame->mdinfo, stream);
        if (ret)
        {
            /* send to encode failed */
            ak_print_error_ex(MODULE_ID_VENC, "send to encode failed\n");
        }
        else        //编码处理成功，TODO：
        {
            //将stream data 通过udp协议传送给另一台机器
            // fwrite(stream->data, stream->len, 1, save_fp);
            // ak_thread_sem_wait(&enc_sem); //等待UDP发送完毕
            ak_print_normal(MODULE_ID_VENC, "venc successed! stream size is %d\n", stream->len);
            ak_venc_release_stream(venc_th->venc_handle, stream);
        }
    }
    ak_mem_free(stream);
    return NULL;
}


void venc_set_param(void)
{
    type = "h264";  //"编码输出数据格式 h264 h265 jpeg "
    main_res = 4;   //"主通道分辨率，0-5"
    sub_res = 1;    //"次通道分辨率，0-3 need smaller than main channel"
    chn_index = 0;  //"vi channel index, 0-main, 1-sub"
}

int venc_init(void)
{
    /* get the type for output */
    if (type == NULL)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the type\n");
        /* print the hint and notice */
        return FAILED;
    }
    /* check the chn index value */
    if (chn_index < 0 || chn_index > 1)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the correct param\n");
        /* print the hint and notice */
        return FAILED;
    }
    /* get the type for input */
    int encoder_type = 0;
    if(!strcmp(type,"h264"))
    {
        /* h264 */
        ak_print_normal(MODULE_ID_VDEC, "h264 success\n");
        encoder_type = H264_ENC_TYPE;
    }
    else if(!strcmp(type,"h265"))
    {
        /* h265 */
        ak_print_normal(MODULE_ID_VDEC, "h265 success\n");
        encoder_type = HEVC_ENC_TYPE;
    }
    else if(!strcmp(type,"jpeg"))
    {
        /* jpeg */
        ak_print_normal(MODULE_ID_VDEC, "jpeg success\n");
        encoder_type = MJPEG_ENC_TYPE;
    }
    else
    {
        ak_print_normal_ex(MODULE_ID_VDEC, "unsupport video enccode input type [%s] \n", type);
        return FAILED;
    }

    /* get param from cmd line */
    int ret = -1;
    int width = resolutions[main_res].width;
    int height = resolutions[main_res].height;
    int subwidth = resolutions[sub_res].width;;
    int subheight = resolutions[sub_res].height;
    int handle_id = -1;

    /* open venc */
    struct venc_param ve_param;

    if (chn_index == 0)
    {
        ve_param.width  = width;            //resolution width
        ve_param.height = height;           //resolution height
    }
    else
    {
        ve_param.width  = subwidth;            //resolution width
        ve_param.height = subheight;           //resolution height
    }

    ve_param.fps    = 20;               //fps set
    ve_param.goplen = 50;               //gop set
    ve_param.target_kbps = 800;         //k bps
    ve_param.max_kbps    = 1024;        //max kbps
    ve_param.br_mode     = BR_MODE_CBR; //br mode
    ve_param.minqp       = 25;          //qp set
    ve_param.maxqp       = 50;          //qp max value
    ve_param.initqp       = (ve_param.minqp + ve_param.maxqp)/2;    //qp value
    ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;     //jpeg qlevel
    ve_param.chroma_mode = CHROMA_4_2_0;            //chroma mode
    ve_param.max_picture_size = 0;                  //0 means default
    ve_param.enc_level        = 30;                 //enc level
    ve_param.smart_mode       = 0;                  //smart mode set
    ve_param.smart_goplen     = 100;                //smart mode value
    ve_param.smart_quality    = 50;                 //quality
    ve_param.smart_static_value = 0;                //value
    ve_param.enc_out_type = encoder_type;           //enc type

    /* decode type is h264*/
    if(encoder_type == H264_ENC_TYPE)
    {
        /* profile type */
        ve_param.profile     = PROFILE_MAIN;
        
    }
    /*decode type is jpeg*/
    else if(encoder_type == MJPEG_ENC_TYPE)
    {
        /* jpeg enc profile */
        ve_param.initqp = 80;
        ve_param.profile = PROFILE_JPEG;
    }
    /*decode type is h265*/
    else
    {
        /* hevc profile */
        ve_param.profile = PROFILE_HEVC_MAIN;
    }

    ret = ak_venc_open(&ve_param, &handle_id);

    if (ret || (-1 == handle_id) )
    {
        ak_print_error_ex(MODULE_ID_VENC, "open venc failed\n");
        return FAILED;
    }

    /* recode the file */
    enc_pair.venc_handle = handle_id;
    enc_pair.en_type     = encoder_type;

    //信号量初始化
    ret = ak_thread_sem_init(&enc_sem, 0);
    if (ret)
    {
        ak_print_error_ex(MODULE_ID_VENC, "thread sem init failed");
        return FAILED;
    }
    return SUCCESS;
}

void venc_start(void)
{
        /* create the venc thread */
    ak_thread_create(&venc_stream_th, video_encode_from_vi_th, &enc_pair, ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);

    /* WAITER for the thread exit */
    ak_thread_join(venc_stream_th);
}

void venc_close(void)
{
    if(enc_pair.venc_handle != -1)
    {
        /* close the venc*/
        ak_venc_close(enc_pair.venc_handle);
    }
}

//信号量加一，即启动线程开始处理
void venc_thread_sem_post(void)
{
    ak_thread_sem_post(&enc_sem);
}


void enc_pair_set_source(struct video_input_frame *frame)
{
    enc_pair.frame = frame;
}
