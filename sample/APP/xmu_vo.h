#ifndef XMU_VO_H
#define XMU_VO_H
#include "DataBuff.h"
#include "BaseThread.h"
extern "C"{
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_graphics.h"
#include "ak_vdec.h"
#include "ak_vo.h"
#include "ak_thread.h"
#include "ak_mem.h"
}

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

/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {1920,	1080,  "DE_VIDEO_SIZE_1080P"},
    {2560,  1920,  "DE_VIDEO_SIZE_1920P"}
};

class Vo
{

public:
    Vo();
    ~Vo();
    static int demo_play_func(struct ak_vdec_frame *frame);
    static int set_obj_pos(int decoder, int max_width, int max_height);
    static void decode_stream(int handle_id, unsigned char *data, int read_len);
private:
    void set_param();
    int init();
public:
    DataBuffer dbf;

    int decoder_num;     //"解码的数�?：[1-4]"
    char *type;   //"解码数据格式 val：h264 h265 jpeg"
    int res;

    char *pc_prog_name;                      //demo名称
    //char *file;                              //file name
    int screen;                         //mipi屏幕
	//多线程同步
    int refresh_flag;                         //flag to refresh
    int refresh_record_flag;                    //flag to refresh
    //int handle_id[MAX_DE_NUM];  //vdec handle id
    ak_mutex_t refresh_flag_lock;
    struct ak_layer_pos  obj_pos[MAX_DE_NUM];              //store the pos

};



#endif