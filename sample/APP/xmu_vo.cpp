#include "xmu_vo.h"
#include <string.h>


Vo::Vo(/* args */)
{
	set_param();
	init();
}

Vo::~Vo()
{
    /* destroy the layer and close vo */
    ak_vo_destroy_layer(AK_VO_LAYER_VIDEO_1);
    ak_vo_close(DEV_NUM);
    ak_thread_mutex_destroy(&refresh_flag_lock);
}

void Vo::set_param()
{
    decoder_num = 1;     //"解码的数�?：[1-4]"
    type = "h264";   //"解码数据格式 val：h264 h265 jpeg"
    res = 3;  //"码流分辨率，0 - 640*360, 1 - 640*480, 2 - 1280*720, 3 - 1920*1080, 4 - 2560*1920
    pc_prog_name = NULL;                      //demo名称
    screen         = 0;                         //mipi屏幕
    refresh_flag   = 0;                         //flag to refresh
    refresh_record_flag = 0;                    //flag to refresh
    // handle_id[MAX_DE_NUM] = { -1, -1, -1, -1};  //vdec handle id

}

int Vo::init()
{
    /* SUPPORT the 4 decoder open */
    if (decoder_num > MAX_DE_NUM || decoder_num < 1)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "Only support the max [%d] decoder\n", MAX_DE_NUM);
        return AK_FAILED;
    }

    /* SUPPORT the 1 decoder open 2560*1920 */
    if (decoder_num > 1 && (res == DE_VIDEO_SIZE_MAX-1))
    {
        ak_print_error_ex(MODULE_ID_VDEC, "Only support 1 decoder for 2560*1920\n");
        return AK_FAILED;
    }

    /* screen res */
    int dst_width = 0;
    int dst_height = 0;
    AK_GP_ROTATE rotate = AK_GP_ROTATE_NONE;
    int ret = 0;
    if (screen == 1)
    {
        /* if use the RGB screen */
        dst_width = RGB_SC_WIDTH;
        dst_height = RGB_SC_HEIGHT;
        rotate    = AK_GP_ROTATE_NONE;
    }
    else if (screen == 0)
    {
        /* if use the MIPI screen */
        dst_width = MIPI_SC_WIDTH;
        dst_height = MIPI_SC_HEIGHT;
        rotate = AK_GP_ROTATE_90;
    }
    else
    {
        ak_print_error_ex(MODULE_ID_VDEC, "check the screen type you input!!!\n");
        return AK_FAILED;
    }

    /* open vo device*/
    struct ak_vo_param vo_param = {0};
    vo_param.width = dst_width;
    vo_param.height = dst_height;
    vo_param.format = GP_FORMAT_RGB888;     //set to RGB888 output
    vo_param.rotate = rotate;

    /* start to open the vo */
    ret = ak_vo_open(&vo_param, DEV_NUM);
    if(ret != 0)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "ak_vo_open failed![%d]\n", ret);
        return ret;
    }

    /* create the video layer */
    struct ak_vo_layer_in video_layer;
    video_layer.create_layer.height = dst_height;   //layer size
    video_layer.create_layer.width  = dst_width;    //layer size
    video_layer.create_layer.left  = 0;             //layer pos 
    video_layer.create_layer.top   = 0;             //layer pos
    video_layer.layer_opt          = GP_OPT_NONE;             //opt
    ret = ak_vo_create_video_layer(&video_layer, AK_VO_LAYER_VIDEO_1);
    if(ret != 0)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "ak_vo_create_video_layer failed![%d]\n", ret);
        ak_vo_close(DEV_NUM);
        return AK_FAILED;
    }

    /* set the obj pos in layer */
    if (set_obj_pos(decoder_num, dst_width, dst_height))
        return AK_FAILED;

	ak_thread_mutex_init(&refresh_flag_lock, NULL);
}


/* this func is used to set the obj pos in layer */
int Vo::set_obj_pos(int decoder, int max_width, int max_height)
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
        return AK_FAILED;
    }

    return AK_SUCCESS;
}

/**
 * decode_stream - send the stream data to decode
 * @adec_handle[IN]: adec handle
 * @data[IN]: audio file data
 * @read_len[IN]: audio file data length
 * notes:
 */
void Vo::decode_stream(int handle_id, unsigned char *data, int read_len)
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

/* show the stream to screen */
int Vo::demo_play_func(struct ak_vdec_frame *frame)
{
    /* set the pos in the screen */
    int pos_top = obj_pos[frame->id].top;
    int pos_left = obj_pos[frame->id].left;
    int width = obj_pos[frame->id].width;
    int height = obj_pos[frame->id].height;

    /* obj to add to layer */
    struct ak_vo_obj    obj;
    memset(&obj, 0x00, sizeof(struct ak_vo_obj));
    if (frame->data_type == AK_TILE_YUV)
    {
        /* if the data type is tileyuv */
        obj.format = GP_FORMAT_TILED32X4;
        obj.vo_tileyuv.data = frame->tileyuv_data;
    }
    else
    {
        /* if the data type is yuv420sp */
        obj.format = GP_FORMAT_YUV420SP;
        obj.cmd = GP_OPT_SCALE;         /* scale to screen */
        obj.vo_layer.width = frame->yuv_data.pitch_width;   /* the real width */
        obj.vo_layer.height = frame->yuv_data.pitch_height; /* the real height */

        /* pos and range from the src */
        obj.vo_layer.clip_pos.top = 0;
        obj.vo_layer.clip_pos.left = 0;
        obj.vo_layer.clip_pos.width = frame->width;
        obj.vo_layer.clip_pos.height = frame->height;
        ak_mem_dma_vaddr2paddr(frame->yuv_data.data, &(obj.vo_layer.dma_addr));
    }

    /* pos and range for the dst layer to contain the src */
    obj.dst_layer.top = pos_top;
    obj.dst_layer.left = pos_left;
    obj.dst_layer.width = width;
    obj.dst_layer.height = height;

    /* add this picture to layer */
    ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);

    ak_thread_mutex_lock(&refresh_flag_lock);
    /* set the refresh flag */
    refresh_flag |= (0x01 << frame->id);

    /* get the value */
    if(refresh_flag == refresh_record_flag)
    {
        refresh_flag = 0;
        /* video_1 is created,only flush the layer */
        int cmd = AK_VO_REFRESH_VIDEO_GROUP & 0x100;
        ak_vo_refresh_screen(cmd);
    }
    ak_thread_mutex_unlock(&refresh_flag_lock);

    /* wait for all decoder ready */
    while ( (refresh_flag != refresh_record_flag) && (refresh_flag & (0x01 << frame->id)) )
    {
        ak_sleep_ms(5);
    }

    return 0;
}

