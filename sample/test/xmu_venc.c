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

struct venc_pair
{
    int venc_handle;
    char *file;      //rtsp client handle
    char save_file[128];
    int en_type;
}enc_pair;

/* resolation define */
struct resolution_t {
	unsigned int width;
	unsigned int height;
	unsigned char str[20];
};

static char *pc_prog_name = NULL;                      //demo名称
static char *file  = NULL;                             //file name
static char *type         = NULL;                      //get the type input
static char *save_path    = "/mnt/video_encode";
static int main_res       = 4;
static int sub_res        = 1;
static int frame_num   = 1000;
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

void *video_encode_from_vi_th(void *arg)
{
    ak_thread_set_name("venc_th");
    struct venc_pair *venc_th = (struct venc_pair *)arg; //venc handle

    /* a dir opt */
    int count = 0;
    FILE *save_fp = NULL;
    struct video_input_frame frame;

    ak_print_normal(MODULE_ID_VI, "capture start\n");

    /* init the save file name and open it */
    memset(venc_th->save_file, 0x00, sizeof(venc_th->save_file));
    if (H264_ENC_TYPE == venc_th->en_type)
        sprintf(venc_th->save_file, "%s/h264_chn%d_%d.str", save_path, chn_index, venc_th->venc_handle);
    else
        sprintf(venc_th->save_file, "%s/h265_chn%d_%d.str", save_path, chn_index, venc_th->venc_handle);

    /* save file open */
    if (MJPEG_ENC_TYPE != venc_th->en_type)
        save_fp = fopen(venc_th->save_file, "w+");

    /* frame num cal */
    while(count < frame_num)
    {
        memset(&frame, 0x00, sizeof(frame));

        /* if save as jpeg data */
        if (MJPEG_ENC_TYPE == venc_th->en_type)
        {
            memset(venc_th->save_file, 0x00, sizeof(venc_th->save_file));
            sprintf(venc_th->save_file, "%s/chn%d_%d_num_%d.jpeg", save_path, chn_index, venc_th->venc_handle, count);
            save_fp = fopen(venc_th->save_file, "w+");
        }

        /* to get frame */
        VI_CHN chn_id = (chn_index == 0) ?  VIDEO_CHN0 : VIDEO_CHN1;
        int ret = ak_vi_get_frame(chn_id, &frame);
        if (!ret) 
        {
            /* send it to encode */
            struct video_stream *stream = ak_mem_alloc(MODULE_ID_VENC, sizeof(struct video_stream));
            ret = ak_venc_encode_frame(venc_th->venc_handle, frame.vi_frame.data, frame.vi_frame.len, frame.mdinfo, stream);
            if (ret)
            {
                /* send to encode failed */
                ak_print_error_ex(MODULE_ID_VENC, "send to encode failed\n");
            }
            else
            {
                fwrite(stream->data, stream->len, 1, save_fp);
                ak_venc_release_stream(venc_th->venc_handle, stream);
                count++;
            }

            ak_mem_free(stream);
            ak_vi_release_frame(chn_id, &frame);
        }
        else
        {
            /* 
             *	If getting too fast, it will have no data,
             *	just take breath.
             */
            ak_print_normal_ex(MODULE_ID_VI, "get frame failed!\n");
            ak_sleep_ms(10);
        }

        /* save as a jpeg picture */
        if (MJPEG_ENC_TYPE == venc_th->en_type)
        {
            if (NULL != save_fp)
            {
                fclose(save_fp);
                save_fp = NULL;
            }
        }
    }

    /* finished */
    if (MJPEG_ENC_TYPE != venc_th->en_type)
        fclose(save_fp);

    return NULL;
}


void venc_set_param(void)
{
    frame_num = 1000;   //"从vi获取的帧的数量"
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
        help_hint();
        return FAILED;
    }
    /* check the chn index value */
    if (chn_index < 0 || chn_index > 1)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the correct param\n");
        /* print the hint and notice */
        help_hint();
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
    enc_pair.file = file;
    enc_pair.venc_handle = handle_id;
    enc_pair.en_type     = encoder_type;

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