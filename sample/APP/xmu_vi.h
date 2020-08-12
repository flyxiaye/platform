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

struct venc_pair
{
    int venc_handle;
    int en_type;
};

class Vi :public BaseThread
{
public:
    Vi();
    ~Vi();
    void start();
    void run(void);
    DataBuffer dbf;
private:
    void set_param();
    int init();
    int handle_id;
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