#include "xmu_vo.h"
#include <string.h>


Vo::Vo(/* args */)
{
}

Vo::~Vo()
{
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

void Vo::set_param()
{
    decoder_num = 1;     //"解码的数�?：[1-4]"
    type = "jpeg";   //"解码数据格式 val：h264 h265 jpeg"
    res = 3;  //"码流分辨率，0 - 640*360, 1 - 640*480, 2 - 1280*720, 3 - 1920*1080, 4 - 2560*1920
    pc_prog_name = NULL;                      //demo名称
    file  = NULL;                              //file name
    screen         = 0;                         //mipi屏幕
    refresh_flag   = 0;                         //flag to refresh
    refresh_record_flag = 0;                    //flag to refresh
    // handle_id[MAX_DE_NUM] = { -1, -1, -1, -1};  //vdec handle id
    ak_mutex_t refresh_flag_lock;
    struct ak_layer_pos  obj_pos[MAX_DE_NUM];              //store the pos

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

void Vo::start(){
    enum ak_vdec_input_type intype = AK_CODEC_H264;
    enum ak_vdec_output_type outtype = AK_YUV420SP;
    int mode = 0;

    if (type == NULL)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "please input the type\n");
        /* print the hint and notice */
        return;
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
        ak_print_normal_ex(MODULE_ID_VDEC, "unsupport video decode input type [%s] \n", type);

    /* output vdec data set */
    if(intype != AK_CODEC_MJPEG)
        outtype = AK_TILE_YUV;

    /* open vdec */
    struct ak_vdec_param param;
    param.vdec_type = intype;
    param.sc_height = resolutions[res].height;         //vdec height res set
    param.sc_width = resolutions[res].width;          //vdec width res set
    param.output_type = outtype;

    /* open the vdec */
    int ret = ak_vdec_open(&param, &handle_id[i]);
    if(ret != 0)
    {
        ak_print_error_ex(MODULE_ID_VDEC, "ak_vdec_open failed!\n");

        /* destroy the layer */
        return;
    }
    
    ak_thread_mutex_lock(&refresh_flag_lock);
    /* refresh flag to record */
    refresh_record_flag |= (1 << handle_id[i]);
    ak_thread_mutex_unlock(&refresh_flag_lock);

    /* mode == 1 means the solo frame decode */
    if (mode)
    {   
        /* if decode the jpeg data */
        ak_pthread_t jpeg_stream_th;
        ak_thread_create(&jpeg_stream_th, vdec_solo_frame, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
    }
    else
    {
        /* h264 or h265 decode, there is two thread */
        /* get frame thread */
        ak_pthread_t tid_stream_th;
        ak_thread_create(&tid_stream_th, vdec_get_frame_th, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);

        /* send stream thread */
        ak_pthread_t pid;
        ak_thread_create(&pid, vdec_send_decode_th, &handle_id[i], ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);	
    }
    i++;

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

void Vo::H264_1::run(void)
{
    //send to decode
    ak_thread_set_name("vdec_demo_test");

    // int *handle_id = (int *)arg;
    // char *file_name = file;

    /* read video file stream and send to decode, */
    int read_len = 0;
    int total_len = 0;
    unsigned char *data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, RECORD_READ_LEN);

    /* loop for sending data to decode */
    do
    {
        /* read the record file stream */
        memset(data, 0x00, RECORD_READ_LEN);
        read_len = fread(data, sizeof(char), RECORD_READ_LEN, fp);
        
        /* get the data and send to decoder */
        if(read_len > 0)
        {
            total_len += read_len;
            /* play loop */
            decode_stream(*handle_id, data, read_len);
            ak_sleep_ms(10);
        }
        else if(0 == read_len)
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
    if (data != NULL)
        ak_mem_free(data);
#ifndef AK_RTOS
    posix_fadvise(fileno(fp), 0, total_len, POSIX_FADV_DONTNEED);
#endif
    fclose(fp);

    ak_print_normal_ex(MODULE_ID_VDEC, "send stream th exit\n");
    return NULL;
}

void Vo::Jpeg::run(void)
{
    int ret = -1;

    /* read video file stream and send to decode, */
    int read_len = 0;
    int total_len = 0;
    unsigned char *data = NULL;

    /* a loop for read all the jpeg file */
    data = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, file_size);

    /* read the record file stream */
    memset(data, 0x00, sizeof(data));
    read_len = fread(data, sizeof(char), file_size, fp);

    /* send the data to decode */
    ak_print_normal(MODULE_ID_VDEC, "size is [%d] \n", read_len);
    if(read_len > 0)
    {
        total_len += read_len;
        /* play loop */
        decode_stream(*handle_id, data, read_len);
        ak_sleep_ms(10);
    }
    else if(0 == read_len)
    {
        /* read to the end of file */
        ak_print_normal_ex(MODULE_ID_VDEC, "\n\tread to the end of file\n");
        goto file_failed;
    }
    else 
    {
        /* error is not support */
        ak_print_error_ex(MODULE_ID_VDEC, "read file error\n");
        goto file_failed;
    }

    /* inform that sending data is over */
    ak_vdec_end_stream(*handle_id);

    struct ak_vdec_frame frame;
    /* need to get the frame for a while */
    while (1)
    {
        /* get the frame */
        ret = ak_vdec_get_frame(*handle_id, &frame);
        if(ret == 0)
        {
            /* set frame status */
            demo_play_func(&frame);

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


    if (NULL != data)
        ak_mem_free(data);

        
    


    ak_print_normal_ex(MODULE_ID_VDEC, "JPEG_sending is done, decoder [%d] send jpeg th exit\n", *handle_id);
    // decoder_num--;
    // ak_thread_mutex_lock(&refresh_flag_lock);
    // //clear_bit
    // refresh_record_flag &= ~(1 << (*handle_id));
    // refresh_flag &= ~(1 << (*handle_id));
    // ak_thread_mutex_unlock(&refresh_flag_lock);
    // return NULL;
}

void Vo::H264_2::run(void)
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
            /* invoke the callback function to process the frame*/
            demo_play_func(&frame);

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

static void * callback_jpeg(void *arg)
{
    ((Vo::Jpeg*)arg)->run();
}

static void * callback_h264_1(void *arg)
{
    ((Vo::H264_1*)arg)->run();
}

static void * callback_h264_2(void *arg)
{
    ((Vo::H264_2*)arg)->run();
}

void Vo::Jpeg::start()
{
    BaseThread::start(callback_jpeg);
}

void Vo::H264_1::start()
{
    BaseThread::start(callback_h264_1);
}

void Vo::H264_2::start()
{
    BaseThread::start(callback_h264_2);
}