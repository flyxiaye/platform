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

// #define DE_VIDEO_SIZE_MAX    5
// #define MAX_DE_NUM   4


/* resolation define */
struct resolution_t {
	unsigned int width;
	unsigned int height;
	unsigned char str[20];
};

static struct resolution_t resolutions[5] = {
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
    int demo_play_func(struct ak_vdec_frame *frame);
    int set_obj_pos(int decoder, int max_width, int max_height);
    void decode_stream(int handle_id, unsigned char *data, int read_len);
private:
    void set_param();
    int init();
public:
    enum {
        MAX_DE_NUM = 4,
        DE_VIDEO_SIZE_MAX = 5
    };
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
    struct ak_layer_pos obj_pos[MAX_DE_NUM];              //store the pos
};




#endif