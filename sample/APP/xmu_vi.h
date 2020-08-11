#ifndef XMU_VI_H
#define XMU_VI_H
#include "BaseThread.h"
#include "DataBuff.h"
#include "xmu_common.h"

extern "C"{
#include "ak_common.h"
#include "ak_common_graphics.h"
#include "ak_common_video.h"
#include "ak_vo.h"
#include "ak_vi.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_tde.h"
#include "ak_venc.h"
}

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

struct venc_pair
{
    int venc_handle;
    int en_type;
};

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

class Vi
{
public:
    Vi();
    ~Vi();
    void start();
    void run(void);
private:
    void set_param();
    int init();
    
    DataBuffer dbf;

    int channel_num;
    char *isp_path;
    // int main_res_id = 4;
    // int sub_res_id = 0;
    char *pc_prog_name;                      //demo名称
    char *type;                      //get the type input
    int main_res;
    int sub_res;
    // static char *cfg = "/etc/jffs2/isp_pr2000_dvp.conf";
    ak_mutex_t refresh_flag_lock;
    int chn_index;

    struct venc_pair enc_pair;
};


#endif