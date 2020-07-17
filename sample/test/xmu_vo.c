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

#include "ak_common_graphics.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_vo.h"
#include "ak_tde.h"

//============自定义变量和宏定义===========//
#include "xmu_common.h"
struct output_param {
    int dst_height;
    int dst_width;
    int dou;
    int len;
    int rotate;
} vo_param;
//============自定义变量和宏定义end===========//

#define MIPI_SC_WIDTH 1280
#define MIPI_SC_HEIGHT 800

#define RGB_SC_WIDTH  1024
#define RGB_SC_HEIGHT  600

#define LEN_HINT                512
#define LEN_OPTION_SHORT        512
#define MAX_DIS_NUM             4

static char *pc_prog_name = NULL;                                                      //demo名称
static int screen_flag    = 0;                                                         //mipi屏幕
static char *logo_file = "/mnt/anyka.logo.577.160.rgb";
static int display_num = 4;
static AK_GP_FORMAT data_format = GP_FORMAT_YUV420P;
static AK_GP_FORMAT out_format  = GP_FORMAT_RGB888;


void vo_set_param(void)
{
    screen_flag = 1;    //"显示屏幕类型 0 - MIPI, 1 - RGB" 
	display_num = 1;    //"分屏数目 [1, 4]" 
	data_format = 5;    //"[NUM] 数据输入格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP 6:ARGB8888 7:RGBA8888 8:ABGR8888 9:BGRA8888 10:TILED32X4" 
	out_format = 1;     //"[NUM] 数据输出格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 "
	logo_file = "/mnt/anyka.logo.577.160.rgb"; //"logo 文件的路径，只支持提供的anyka.logo.577.160.rgb文件显示"
}

int vo_init(void)
{

	if (display_num < 0 || display_num > MAX_DIS_NUM)
	{
		ak_print_error_ex(MODULE_ID_VO, "display num is only support 0 to %d\n", MAX_DIS_NUM);
		return FAILED;
	}

	int ret = 0;
	/* get the res and rotation */

	/* get the screen res */
	if (screen_flag == 1)
	{
		/* 1 means for RGB */
		vo_param.dst_width = RGB_SC_WIDTH;
		vo_param.dst_height = RGB_SC_HEIGHT;
		vo_param.rotate = AK_GP_ROTATE_NONE;
	}
	else if (screen_flag == 0)
	{
		/* 0 means for MIPI */
		vo_param.dst_width = MIPI_SC_WIDTH;
		vo_param.dst_height = MIPI_SC_HEIGHT;
		vo_param.rotate = AK_GP_ROTATE_90;
	}
	else
	{
		ak_print_error_ex(MODULE_ID_VO, "screen type is invalid\n");
		return FAILED;
	}

	/* fill the struct to open the vo */
	struct ak_vo_param	param;

	/* fill the struct ready to open */
	param.width = vo_param.dst_width;//1280;
	param.height = vo_param.dst_height;// 800;
	param.format = out_format;  //format to output
	param.rotate = vo_param.rotate;    //rotate value

	/* open vo */
	ret = ak_vo_open(&param, DEV_NUM);      //open vo

	if (ret != 0)
	{
		/* open failed return -1 */
		ak_print_error_ex(MODULE_ID_VO, "ak_vo_open failed![%d]\n", ret);
		return FAILED;
	}

	/* create the video layer */
	struct ak_vo_layer_in video_layer;
	video_layer.create_layer.height = vo_param.dst_height;   //800; //get the res
	video_layer.create_layer.width = vo_param.dst_width;    //1280; //get the res
	/* just from (0,0) */
	video_layer.create_layer.left = 0;         //layer left pos
	video_layer.create_layer.top = 0;         // layer top pos
	video_layer.layer_opt = 0;
	video_layer.format = out_format;

	ret = ak_vo_create_video_layer(&video_layer, AK_VO_LAYER_VIDEO_1);
	if (ret != 0)
	{
		/* if failed, close the vo */
		ak_print_error_ex(MODULE_ID_VO, "ak_vo_create_video_layer failed![%d]\n", ret);
		ak_vo_close(DEV_NUM);       // if failed, close the vo
		return FAILED;
	}

	/* create the gui layer */
	struct ak_vo_layer_in gui_layer;
	/* only support the given log file */
	gui_layer.create_layer.height = 160;    /* logo res */
	gui_layer.create_layer.width = 577;    /* logo res */
	gui_layer.create_layer.left = 0;       /* logo pos */
	gui_layer.create_layer.top = 0;       /* logo pos */
	gui_layer.format = out_format;  /* gui layer format */

	gui_layer.layer_opt = GP_OPT_COLORKEY; /* support the colorkey opt */

	gui_layer.colorkey.coloract = COLOR_DELETE;       /* delete the color */
	gui_layer.colorkey.color_min = 0xFFFF00;        /* min value */
	gui_layer.colorkey.color_max = 0xFFFFFF;        /* max value */

	struct ak_vo_layer_out gui_info;                /* output the gui layer info */
	ret = ak_vo_create_gui_layer(&gui_layer, AK_VO_LAYER_GUI_1, &gui_info);
	if (ret != 0)
	{
		ak_print_error_ex(MODULE_ID_VO, "ak_vo_create_gui_layer failed![%d]\n", ret);
		ak_vo_destroy_layer(AK_VO_LAYER_VIDEO_1);
		ak_vo_close(DEV_NUM);
		return FAILED;
	}

	/* use the double buff mode */
	ak_vo_set_fbuffer_mode(AK_VO_BUFF_DOUBLE);

	/* support the yuv 420p */
	vo_param.len = (1920 * 1080 * 3) / 2;

	/* get the size of the pixel */
	switch (data_format)
	{
		/* 565 */
	case GP_FORMAT_RGB565:
	case GP_FORMAT_BGR565:
		vo_param.len = 1920 * 1080 * 2;
		break;
		/* 888 */
	case GP_FORMAT_BGR888:
	case GP_FORMAT_RGB888:
		vo_param.len = 1920 * 1080 * 3;
		break;
		/* 8888 */
	case GP_FORMAT_ABGR8888:
	case GP_FORMAT_ARGB8888:
	case GP_FORMAT_RGBA8888:
	case GP_FORMAT_BGRA8888:
		vo_param.len = 1920 * 1080 * 4;
		break;
		/* YUV */
	case GP_FORMAT_YUV420P:
	case GP_FORMAT_YUV420SP:
		break;
	default:
		ak_print_error_ex(MODULE_ID_VO, "here not support the tileyuv data\n");

	}
    return SUCCESS;
}

//传入图像数据，并输出
void vo_put_one_frame(void *data)
{
    int ret = 0;
    memset(data, 0, vo_param.len);
    /* obj add */
    struct ak_vo_obj obj;

    /* set obj src info*/	
    obj.format = data_format;
    obj.cmd = GP_OPT_SCALE;
    obj.vo_layer.width = 1920;
    obj.vo_layer.height = 1080;
    obj.vo_layer.clip_pos.top = 0;
    obj.vo_layer.clip_pos.left = 0;
    obj.vo_layer.clip_pos.width = 1920;
    obj.vo_layer.clip_pos.height = 1080;

    ak_mem_dma_vaddr2paddr(data, &(obj.vo_layer.dma_addr));

    /* show as the screen partion set */
    int counter = display_num;
    if (display_num == 1)
    {
        /* set dst_layer 1 info*/
        obj.dst_layer.top = 0;
        obj.dst_layer.left = 0;
        obj.dst_layer.width = vo_param.dst_width;
        obj.dst_layer.height = vo_param.dst_height;
        /* display obj 1*/
        ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
    }
    else
    {
        /* more than one partion */
        if (counter)
        {
            /* set dst_layer 1 info*/
            obj.dst_layer.top = 0;
            obj.dst_layer.left = 0;
            obj.dst_layer.width = vo_param.dst_width/vo_param.dou;    
            obj.dst_layer.height = vo_param.dst_height/vo_param.dou;
            /* display obj 1*/
            ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
            counter--;
        }

        if (counter != 0)
        {
            /* set dst_layer 2 info*/
            obj.dst_layer.top = 0;
            obj.dst_layer.left = vo_param.dst_width/vo_param.dou;
            obj.dst_layer.width = vo_param.dst_width/vo_param.dou;
            obj.dst_layer.height = vo_param.dst_height/vo_param.dou;//400;
            /* display obj 1*/
            ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
            counter--;
        }

        if (counter)
        {
            /* set dst_layer 3 info*/
            obj.dst_layer.top = vo_param.dst_height/vo_param.dou;//400;
            obj.dst_layer.left = 0;
            obj.dst_layer.width = vo_param.dst_width/vo_param.dou;
            obj.dst_layer.height = vo_param.dst_height/vo_param.dou;//400;
            /* display obj 1*/
            ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
            counter--;
        }

        if (counter)
        {
            /* set dst_layer 4 info*/
            obj.dst_layer.top = vo_param.dst_height/vo_param.dou;//400;
            obj.dst_layer.left = vo_param.dst_width/vo_param.dou;
            obj.dst_layer.width = vo_param.dst_width/vo_param.dou;
            obj.dst_layer.height = vo_param.dst_height/vo_param.dou;//400;
            /* display obj 1*/
            ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
            counter--;
        }

    }

    /* flush to screen */
    int cmd = AK_VO_REFRESH_VIDEO_GROUP & 0x100;
    // cmd |= AK_VO_REFRESH_GUI_GROUP & 0x10000;
    ak_vo_refresh_screen(cmd);
}