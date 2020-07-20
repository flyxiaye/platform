#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_graphics.h"
#include "ak_vdec.h"
#include "ak_vo.h"
#include "ak_thread.h"
#include "ak_mem.h"

#include "xmu_common.h"

#ifdef AK_RTOS
#include "rtthread.h"
#define THREAD_PRIO 90
#else
#define THREAD_PRIO -1
#endif

#define RECORD_READ_LEN      1024*100 /* read video file buffer length */
#define DE_VIDEO_SIZE_MAX    5
/* this is length for parse */
#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

/* mipi screen res */
#define MIPI_SC_WIDTH 1280
#define MIPI_SC_HEIGHT 800

/* RGB screen res */
#define RGB_SC_WIDTH  1024
#define RGB_SC_HEIGHT  600

#define MAX_DE_NUM   4

/* resolation define */
struct resolution_t {
	unsigned int width;
	unsigned int height;
	unsigned char str[20];
};

static char *pc_prog_name = NULL;                      //demo名称
static char *type         = NULL;                      //get the type input
static int screen         = 0;                         //mipi屏幕
static int refresh_flag   = 0;                         //flag to refresh
static int res            = 3;
static int refresh_record_flag = 0;                    //flag to refresh
static int decoder_num = 1;
static int handle_id[MAX_DE_NUM] = { -1, -1, -1, -1};  //vdec handle id
ak_mutex_t refresh_flag_lock;
struct ak_layer_pos  obj_pos[MAX_DE_NUM];              //store the pos

/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {1920,	1080,  "DE_VIDEO_SIZE_1080P"},
    {2560,  1920,  "DE_VIDEO_SIZE_1920P"}
};

//=================my variable===============//
static ak_sem_t dec_sem[MAX_DE_NUM];
struct ak_vdec_param param = {0};
static int mode = 0;
static struct 
{
    unsigned char *data;
    int data_len;
}vdec_data = {NULL, 0};

struct ak_vdec_frame frame;
//==============my variable end===============//

/**
 * decode_stream - send the stream data to decode
 * @adec_handle[IN]: adec handle
 * @data[IN]: audio file data
 * @read_len[IN]: audio file data length
 * notes:
 */
static void decode_stream(int handle_id, unsigned char *data, int read_len)
{
    int send_len = 0;
    int dec_len = 0;
    int ret = 0;

    /* cycle to send the data to decode */
    while (read_len > 0)
    {
        /* send stream to decode */
        ret = ak_vdec_send_stream(handle_id, &data[send_len], read_len, 1, &dec_len);
        if (ret !=  0)
        {
            ak_print_error_ex(MODULE_ID_VDEC, "write video data failed!\n");
            break;
        }

        /* record the length to send to decoder */
        read_len -= dec_len;
        send_len += dec_len;
    }
}

void *vdec_send_decode_th(void *arg)
{
    ak_thread_set_name("vdec_demo_test");
    vdec_data.data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, RECORD_READ_LEN);
    vdec_data.data_len = RECORD_READ_LEN;
    int *handle_id = (int *)arg;
    // int read_len = 0;
    int total_len = 0;
    /* loop for sending data to decode */
    do
    {
        /* read the record file stream */
        /* get the data and send to decoder */
        if(vdec_data.data_len > 0)
        {
            total_len += vdec_data.data_len;
            /* play loop */
            decode_stream(*handle_id, vdec_data.data, vdec_data.data_len);
            ak_sleep_ms(10);
        }
        else if(0 == vdec_data.data_len)
        {
            /* read to the end of file */
            ak_print_normal_ex(MODULE_ID_VDEC, "\n\tread to the end of file\n");
            break;
        }
        else 
        {
            /* if the data is wrong */
            ak_print_error_ex(MODULE_ID_VDEC, "\nread file error\n");
            break;
        }
    }while(1);

    /* if finish, notice the decoder for data sending finish */
    ak_vdec_end_stream(*handle_id);
    ak_print_normal_ex(MODULE_ID_VDEC, "send stream th exit\n");
    return NULL;
}

void *vdec_solo_frame(void *arg)
{

    /* read video file stream and send to decode, */
    int read_len = 0;
    int total_len = 0;
    // unsigned char *data = NULL;
    vdec_data.data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, RECORD_READ_LEN);
    vdec_data.data_len = RECORD_READ_LEN;
    while(1)
    {
        /* send the data to decode */
        ak_print_normal(MODULE_ID_VDEC, "size is [%d] \n", vdec_data.data_len);
        if(vdec_data.data_len > 0)
        {
            total_len += vdec_data.data_len;
            /* play loop */
            decode_stream(*handle_id, vdec_data.data, vdec_data.data_len);
            ak_sleep_ms(10);
        }
        else if(0 == vdec_data.data_len)
        {
            /* read to the end of file */
            ak_print_normal_ex(MODULE_ID_VDEC, "\n\tread to the end of file\n");
            break;
        }
        else 
        {
            /* error is not support */
            ak_print_error_ex(MODULE_ID_VDEC, "read file error\n");
            break;
        }

        /* inform that sending data is over */
        ak_vdec_end_stream(*handle_id);

        // struct ak_vdec_frame frame;
        /* need to get the frame for a while */
        while (1)
        {
            /* get the frame */
            int ret = ak_vdec_get_frame(*handle_id, &frame);
            if(ret == 0)
            {
                //TODO:
                /* set frame status */
                // demo_play_func(&frame);
                //thread should wait for data dealing

                /* relase the frame and push back to decoder */
                ak_vdec_release_frame(*handle_id, &frame);
                break;
            }
            else
            {
                ak_print_normal_ex(MODULE_ID_VDEC, "id [%d] get frame failed! waiting 5 ms!\n", *handle_id);
                ak_sleep_ms(5);
            }
        }
    }

    ak_print_normal_ex(MODULE_ID_VDEC, "JPEG_sending is done, decoder [%d] send jpeg th exit\n", *handle_id);
    decoder_num--;
    ak_thread_mutex_lock(&refresh_flag_lock);
    //clear_bit
    refresh_record_flag &= ~(1 << (*handle_id));
    refresh_flag &= ~(1 << (*handle_id));
    ak_thread_mutex_unlock(&refresh_flag_lock);
    return NULL;
}

void *vdec_get_frame_th(void *arg)
{
    ak_thread_set_name("vdec_get_frame");
    int *id =(int *)arg;
    int ret = -1;
    int status = 0;

    /* a loop for getting the frame and display */
    do{
        /* get frame */
        struct ak_vdec_frame frame;
        ret = ak_vdec_get_frame(*id, &frame);

        if(ret == 0)
        {
            //TODO:
            /* invoke the callback function to process the frame*/
            // demo_play_func(&frame);
            ak_print_normal(MODULE_ID_VDEC, "get_frame_success, frame res is %d*%d\n", frame.width, frame.height);
            /* relase the frame and push back to decoder */
            ak_vdec_release_frame(*id, &frame);
        }
        else
        {
            /* get frame failed , sleep 10ms before next cycle*/
            ak_print_normal_ex(MODULE_ID_VDEC, "id [%d] get frame failed! waiting for 10 ms\n", *id);
            ak_sleep_ms(10);
        }

        /* check the status finished */
        ak_vdec_get_decode_finish(*id, &status);
        /* true means finished */
        if (status)
        {
            decoder_num--;  //that means current decoder is finished
            ak_thread_mutex_lock(&refresh_flag_lock);
            //clear_bit
            refresh_record_flag &= ~(1 << *id);
            refresh_flag &= ~(1 << *id);
            ak_thread_mutex_unlock(&refresh_flag_lock);
            ak_print_normal(MODULE_ID_VDEC, "id is [%d] status [%d]\n", *id, status);
            return NULL;
        }
    }while(1);
}

/* this func is used to set the obj pos in layer */
static int set_obj_pos(int decoder, int max_width, int max_height)
{
    switch (decoder)
    {
    case 1:
        obj_pos[0].top = 0;
        obj_pos[0].left = 0;
        obj_pos[0].width = max_width;
        obj_pos[0].height = max_height;
        break;
    case 2:
    case 3:
    case 4:
        obj_pos[0].top = 0;
        obj_pos[0].left = 0;
        obj_pos[0].width = max_width/2;
        obj_pos[0].height = max_height/2;

        obj_pos[1].top = 0;
        obj_pos[1].left = max_width/2;
        obj_pos[1].width = max_width/2;
        obj_pos[1].height = max_height/2;

        obj_pos[2].top = max_height/2;
        obj_pos[2].left = 0;
        obj_pos[2].width = max_width/2;
        obj_pos[2].height = max_height/2;

        obj_pos[3].top = max_height/2;
        obj_pos[3].left = max_width/2;
        obj_pos[3].width = max_width/2;
        obj_pos[3].height = max_height/2;
        break;
    default :
        return FAILED;
    }

    return SUCCESS;
}

void vdec_set_param(void)
{
    /*"eg.: %s -t 0 -n 1 -f /mnt/test.str -i h264 -r 1920*1080\n";
    "打印帮助信息" ,
    "屏幕类型 0 - mipi, 1 - rgb",
    "解码的数�?：[1-4]",
    "解码视频文件路径或JPEG文件目录路径" ,
    "解码数据格式 val：h264 h265 jpeg",
    "码流分辨率，max�?560*1920";*/
    decoder_num = 1;     //"解码的数�?：[1-4]"
    type = "h264";   //"解码数据格式 val：h264 h265 jpeg"
    res = 3;  //"码流分辨率，0 - 640*360, 1 - 640*480, 2 - 1280*720, 3 - 1920*1080, 4 - 2560*1920
}

int vdec_init(void)
{
    /* SUPPORT the 4 decoder open */
    if (decoder_num > MAX_DE_NUM || decoder_num < 1)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "Only support the max [%d] decoder\n", MAX_DE_NUM);
        return FAILED;
    }

    /* SUPPORT the 1 decoder open 2560*1920 */
    if (decoder_num > 1 && (res == DE_VIDEO_SIZE_MAX-1))
    {
        ak_print_error_ex(MODULE_ID_VDEC, "Only support 1 decoder for 2560*1920\n");
        return FAILED;
    }

    ak_thread_mutex_init(&refresh_flag_lock, NULL);

    int intype = 0;
    int outtype = 0;

    /* get the type for output */
    if (type == NULL)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the type\n");
        return FAILED;
    }

    /* get the type for input */
    if(!strcmp(type,"h264"))
    {
        /* h264 */
        ak_print_normal(MODULE_ID_VDEC, "h264 success\n");
        intype = AK_CODEC_H264;
    }
    else if(!strcmp(type,"h265"))
    {   
        /* h265 */
        ak_print_normal(MODULE_ID_VDEC, "h265 success\n");
        intype = AK_CODEC_H265;
    }
    else if(!strcmp(type,"jpeg"))
    {
        /* jpeg */
        intype = AK_CODEC_MJPEG;
        outtype = AK_YUV420SP;
        mode = 1;
    }
    else
    {
        ak_print_normal_ex(MODULE_ID_VDEC, "unsupport video decode input type [%s] \n", type);
        return FAILED;
    }

    /* output vdec data set */
    if(intype != AK_CODEC_MJPEG)
        outtype = AK_TILE_YUV;

    /* open vdec */
    // struct ak_vdec_param param = {0};
    param.vdec_type = intype;
    param.sc_height = resolutions[res].height;         //vdec height res set
    param.sc_width = resolutions[res].width;          //vdec width res set
    param.output_type = outtype;
    
    //sem init
    for (int k = 0; k < decoder_num; k++)
    {
        int ret = ak_thread_sem_init(&dec_sem[k], 1);
        if (ret)
        {
            ak_print_error_ex(MODULE_ID_VENC, "dec thread sem init failed");
            return FAILED;
        }
    }
    return SUCCESS;
}

void vdec_start(void)
{
    int handleid = 1;
    int i = 0;
    param.vdec_type = AK_CODEC_MJPEG;
    param.sc_height = 1080;         //vdec height res set
    param.sc_width = 1920;          //vdec width res set
    param.output_type = AK_YUV420SP;
    ak_print_normal(MODULE_ID_VDEC, "param is %d, %d\n", param.sc_height, param.sc_width);
    while (i < decoder_num)
    {
        /* open the vdec */
        // int ret = ak_vdec_open(&param, &handle_id[i]);
        int ret = ak_vdec_open(&param, &handleid);
        ak_print_normal(MODULE_ID_VDEC, "handle id is %d\n", handle_id[i]);
        if(ret != 0)
        {
            ak_print_error_ex(MODULE_ID_VDEC, "ak_vdec_open failed!\n");

            /* destroy the layer */
            return;
        }
        
        // ak_thread_mutex_lock(&refresh_flag_lock);
        // /* refresh flag to record */
        // refresh_record_flag |= (1 << handle_id[i]);
        // ak_thread_mutex_unlock(&refresh_flag_lock);

        // /* mode == 1 means the solo frame decode */
        // if (mode)
        // {   
        //     /* if decode the jpeg data */
        //     ak_pthread_t jpeg_stream_th;
        //     ak_thread_create(&jpeg_stream_th, vdec_solo_frame, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
        // }
        // else
        // {
        //     /* h264 or h265 decode, there is two thread */
        //     /* get frame thread */
        //     ak_pthread_t tid_stream_th;
        //     ak_thread_create(&tid_stream_th, vdec_get_frame_th, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);

        //     /* send stream thread */
        //     ak_pthread_t pid;
        //     ak_thread_create(&pid, vdec_send_decode_th, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);	
        // }
        i++;
    }
}

void vdec_close(void)
{

    /* close the vdec and vo, release the src */
    int i = 0;
    while (i < MAX_DE_NUM && handle_id[i] != -1)
    {
        ak_vdec_close(handle_id[i]);
        i++;
    }

    /* destroy the layer and close vo */
    ak_vo_destroy_layer(AK_VO_LAYER_VIDEO_1);
    ak_vo_close(DEV_NUM);
    ak_thread_mutex_destroy(&refresh_flag_lock);
}

void vdec_set_vdec_data(unsigned char *data, int data_len)
{
    vdec_data.data_len = data_len;
    vdec_data.data = data;
}

void vdec_thread_sem_post(void)
{

}

void vdec_open_test()
{
    struct ak_vdec_param param = {0};
    int handleid = 0;
    param.vdec_type = AK_CODEC_MJPEG;
    param.sc_height = 1080;         //vdec height res set
    param.sc_width = 1920;          //vdec width res set
    param.output_type = AK_YUV420SP;
    ak_vdec_open(&param, &handleid);
    ak_print_normal(MODULE_ID_VDEC, "open ok\n");
}