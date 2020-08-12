#include "xmu_vi.h"
#include <memory>
#include <memory.h>
#include <iostream>

#ifdef AK_RTOS
#include "rtthread.h"
#define THREAD_PRIO 90
#else
#define THREAD_PRIO -1
#endif

#define RECORD_READ_LEN      4096*10 /* read video file buffer length */
#define DE_VIDEO_SIZE_MAX    6
/* this is length for parse */
#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

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

/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {960,  1080,   "DE_VIDEO_SIZE_960"},
    {1920,	1080,  "DE_VIDEO_SIZE_1080P"},
    {2560,  1920,  "DE_VIDEO_SIZE_1920P"}
};

Vi::Vi()
{
    this->set_param();
    this->init();
}

Vi::~Vi()
{
    ak_venc_close(enc_pair.venc_handle);
    ak_vi_disable_chn(VIDEO_CHN0);
    ak_vi_disable_chn(VIDEO_CHN1);
    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV0);
}


void Vi::set_param()
{
    channel_num = 0;        //采集通道[0 1], 主通道0，次通道1
	isp_path = (char *)"/etc/isp_ar0230_dvp.conf"; //ISP config file 保存路径, 默认为空,需要填写
	// main_res_id = 4;        //主通道分辨率index[0 - 4]
	// sub_res_id = 0;         //次通道分辨率index[0 - 4]
    pc_prog_name = NULL;                      //demo名称
    type         = (char *)"h264";                      //get the type input
    main_res       = 1;
    sub_res        = 0;
    // static char *cfg = "/etc/jffs2/isp_pr2000_dvp.conf";
    chn_index = 0;
}

int Vi::init()
{
    /* check the res */
    if (main_res < 0 || main_res > DE_VIDEO_SIZE_MAX-1 
            || sub_res < 0 || sub_res > DE_VIDEO_SIZE_MAX-1 
                || main_res < sub_res)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "resolution out of range, check it\n");
        return AK_FAILED;
    }

    /* get the type for output */
    if (type == NULL)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the type\n");
        /* print the hint and notice */
        return AK_FAILED;
    }

    /* check the chn index value */
    if (chn_index < 0 || chn_index > 1)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the correct param\n");
        /* print the hint and notice */
        return AK_FAILED;
    }

    /* get the type for input */
    enum encode_output_type encoder_type = H264_ENC_TYPE;
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
        return AK_FAILED;
    }

    /* get param from cmd line */
    int ret = -1;
    int width = resolutions[main_res].width;
    int height = resolutions[main_res].height;
    int subwidth = resolutions[sub_res].width;;
    int subheight = resolutions[sub_res].height;
    // int handle_id = -1;
    // ak_pthread_t venc_stream_th;

    /* step 1: open video input device */
    ret = ak_vi_open(VIDEO_DEV0);
    if (AK_SUCCESS != ret) 
    {
        ak_print_error_ex(MODULE_ID_VI, "vi device open failed\n");	
        return AK_FAILED;
    }

    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(VIDEO_DEV0, isp_path);
    if (AK_SUCCESS != ret)
    {
        ak_print_error_ex(MODULE_ID_VI, "vi device load cfg [%s] failed!\n", isp_path);	
        return AK_FAILED;
    }
    
    /* 
     * step 3: get sensor support max resolution
     */
    RECTANGLE_S venc_res;				//max sensor resolution
    VI_DEV_ATTR	dev_attr;
    dev_attr.dev_id = VIDEO_DEV0;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = width;
    dev_attr.crop.height = height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;

    ret = ak_vi_get_sensor_resolution(VIDEO_DEV0, &venc_res);
    if (ret)
    {
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    } 
    else 
    {
        ak_print_normal_ex(MODULE_ID_VI, "get dev res w:[%d]h:[%d]\n",venc_res.width, venc_res.height);
        dev_attr.crop.width = venc_res.width;
        dev_attr.crop.height = venc_res.height;
    }

    /* 
     * step 4: set vi working parameters 
     * default parameters: 25fps, day mode, auto frame-control
     */
    ret = ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr);
    if (ret) 
    {
        ak_print_error_ex(MODULE_ID_VI, "vi device set device attribute failed!\n");
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }

    /*
     * step 5: set channel attribute
     */
    VI_CHN_ATTR chn_attr;
    chn_attr.chn_id = VIDEO_CHN0;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_depth = 3;
    chn_attr.frame_rate  = 0;
    ret = ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr);
    if (ret) 
    {
        ak_print_error_ex(MODULE_ID_VI, "vi device set channel attribute failed!\n");
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }

    /*
     * step 5: set sub channel attribute
     */
    VI_CHN_ATTR chn_attr_sub;
    chn_attr_sub.chn_id = VIDEO_CHN1;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_depth = 3;
    chn_attr_sub.frame_rate  = 0;
    ret = ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_VI, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }
    ak_print_normal_ex(MODULE_ID_VI, "vi device set sub channel attribute\n");

    /* 
     * step 6: start capture frames
     */
    //TODO: error
    ret = ak_vi_enable_dev(VIDEO_DEV0);
    if (ret)
    {
        ak_print_error_ex(MODULE_ID_VI, "vi device enable device  failed!\n");
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }
    // return AK_SUCCESS;
    /* 
     * step 7: enable vi main channel 
     */
    ret = ak_vi_enable_chn(VIDEO_CHN0);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable failed!\n",VIDEO_CHN0);
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }

    /* 
     * step 7: enable vi sub channel 
     */
    ret = ak_vi_enable_chn(VIDEO_CHN1);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable failed!\n",VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return AK_FAILED;
    }
    // return AK_SUCCESS;
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

    ve_param.fps    = 30;               //fps set
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
    ve_param.smart_mode       = SMART_DISABLE;                  //smart mode set
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
        return AK_FAILED;
    }

    /* recode the file */
    // enc_pair.file = file;
    enc_pair.venc_handle = handle_id;
    enc_pair.en_type     = encoder_type;
}

/* get the encode data from file path */
void Vi::run(void)
{
    ak_thread_set_name("venc_th");
    // struct venc_pair *venc_th = (struct venc_pair *)arg; //venc handle

    /* a dir opt */
    // int count = 0;
    struct video_input_frame frame;
    memset(&frame, 0x00, sizeof(frame));
    ak_print_normal(MODULE_ID_VI, "capture start\n");

    /* frame num cal */
    while(1)
    {
        memset(&frame, 0x00, sizeof(frame));
        VI_CHN chn_id = (chn_index == 0) ?  VIDEO_CHN0 : VIDEO_CHN1;
        int ret = ak_vi_get_frame(chn_id, &frame);
        if (!ret) 
        {
            /* send it to encode */
            struct video_stream *stream = (struct video_stream *)ak_mem_alloc(MODULE_ID_VENC, sizeof(struct video_stream));
            ret = ak_venc_encode_frame(enc_pair.venc_handle, frame.vi_frame.data, frame.vi_frame.len, frame.mdinfo, stream);
            if (ret)
            {
                /* send to encode failed */
                ak_print_error_ex(MODULE_ID_VENC, "send to encode failed\n");
            }
            else
            {
                ak_print_normal(MODULE_ID_VENC, "encode success, len is %d\n", stream->len);
                dbf.rb_write(stream->data, stream->len);
                ak_venc_release_stream(enc_pair.venc_handle, stream);
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

    }}

static void * callback(void * arg)
{
	((Vi*)arg)->run();
}

void Vi::start(void)
{
    BaseThread::start(callback);
}
