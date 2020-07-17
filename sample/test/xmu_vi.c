#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ak_common.h"
#include "ak_common_graphics.h"
#include "ak_vo.h"
#include "ak_vi.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_tde.h"

//============自定义变量和宏定义===========//
#include "xmu_common.h"
//============自定义变量和宏定义end===========//

#define DEF_FRAME_DEPTH		3
#define RES_GROUP	5

static int channel_num = 0;
static char *isp_path = "";
static int main_res_id = 4;
static int sub_res_id = 0;

/* support resolution list */
static RECTANGLE_S  res_group[RES_GROUP]=
{{640, 360},			/* 640*360 */
 {640, 480},			/*   VGA   */
 {1280,720},			/*   720P  */
 {960, 1080},			/*   1080i */
 {1920,1080}			/*   1080P */
};


//param set
void vi_set_param(void)
{
	channel_num = 0;        //采集通道[0 1], 主通道0，次通道1
	isp_path = "/etc/isp_ar0230_dvp.conf"; //ISP config file 保存路径, 默认为空,需要填写
	main_res_id = 4;        //主通道分辨率index[0 - 4]
	sub_res_id = 0;         //次通道分辨率index[0 - 4]
}

//正确返回0, 错误返回1
int vi_init(void)
{
    	/*check param validate*/
	if (channel_num < 0 || channel_num > 2 || strlen(isp_path) == 0)
	{
		ak_print_error_ex(MODULE_ID_VI, "INPUT param error!\n");
		//help_hint(argv[0]);
		return FAILED;
	}

	int ret = -1;								//return value
	int width = res_group[main_res_id].width;
	int height = res_group[main_res_id].height;
	int subwidth = res_group[sub_res_id].width;
	int subheight = res_group[sub_res_id].height;

	/* open vi flow */

	/*
	 * step 1: open video input device
	 */
	ret = ak_vi_open(VIDEO_DEV0);
	if (AK_SUCCESS != ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device open FAILEDed\n");
		return FAILED;
	}

	/*
	 * step 2: load isp config
	 */
	ret = ak_vi_load_sensor_cfg(VIDEO_DEV0, isp_path);
	if (AK_SUCCESS != ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device load isp cfg [%s] FAILEDed!\n", isp_path);
		return FAILED;
	}

	/*
	 * step 3: get sensor support max resolution
	 */
	RECTANGLE_S res;				//max sensor resolution
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

	/* get sensor resolution */
	ret = ak_vi_get_sensor_resolution(VIDEO_DEV0, &res);
	if (ret) {
		ak_print_error_ex(MODULE_ID_VI, "Can't get dev[%d]resolution\n", VIDEO_DEV0);
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}
	else {
		ak_print_normal_ex(MODULE_ID_VI, "get dev res w:[%d]h:[%d]\n", res.width, res.height);
		dev_attr.crop.width = res.width;
		dev_attr.crop.height = res.height;
	}

	/*
	 * step 4: set vi device working parameters
	 * default parameters: 25fps, day mode
	 */
	ret = ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr);
	if (ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device set device attribute FAILEDed!\n");
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}

	/*
	 * step 5: set main channel attribute
	 */
	VI_CHN_ATTR chn_attr;
	chn_attr.chn_id = VIDEO_CHN0;
	chn_attr.res.width = width;
	chn_attr.res.height = height;
	chn_attr.frame_depth = DEF_FRAME_DEPTH;
	/*disable frame control*/
	chn_attr.frame_rate = 0;
	ret = ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr);
	if (ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device set channel [%d] attribute FAILEDed!\n", VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}
	ak_print_normal_ex(MODULE_ID_VI, "vi device main sub channel attribute\n");


	/*
	 * step 6: set sub channel attribute
	 */
	VI_CHN_ATTR chn_attr_sub;
	chn_attr_sub.chn_id = VIDEO_CHN1;
	chn_attr_sub.res.width = subwidth;
	chn_attr_sub.res.height = subheight;
	chn_attr_sub.frame_depth = DEF_FRAME_DEPTH;
	/*disable frame control*/
	chn_attr_sub.frame_rate = 0;
	ret = ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub);
	if (ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device set channel [%d] attribute FAILEDed!\n", VIDEO_CHN1);
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}
	ak_print_normal_ex(MODULE_ID_VI, "vi device set sub channel attribute\n");

	/*
	 * step 7: enable vi device
	 */
	ret = ak_vi_enable_dev(VIDEO_DEV0);
	if (ret) {
		ak_print_error_ex(MODULE_ID_VI, "vi device enable device  FAILEDed!\n");
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}

	/*
	 * step 8: enable vi main channel
	 */
	ret = ak_vi_enable_chn(VIDEO_CHN0);
	if (ret)
	{
		ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable FAILEDed!\n", VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}

	/*
	 * step 9: enable vi sub channel
	 */
	ret = ak_vi_enable_chn(VIDEO_CHN1);
	if (ret)
	{
		ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable FAILEDed!\n", VIDEO_CHN1);
		ak_vi_close(VIDEO_DEV0);
		return FAILED;
	}
	return SUCCESS;
}

//传入视频帧的指针
int vi_get_one_frame(struct video_input_frame *frame, int fream_len)
{
	// struct video_input_frame frame;
	memset(frame, 0x00, fream_len);
	int ret = ak_vi_get_frame(channel_num, frame);
	if (!ret)
		return SUCCESS;
	else
		ak_sleep_ms(10);
		return FAILED;
}

void vi_release_one_frame(struct video_input_frame *frame)
{
	ak_vi_release_frame(channel_num, frame);
}
