#include "xmu_vi.h"
#include "xmu_vo.h"
#include "xmu_venc.h"
#include "xmu_vdec.h"
#include "xmu_common.h"

#include "ak_common.h"
#include "ak_vi.h"
//#include "ak_vo.h"
#include "ak_log.h"

void test3(void)	//多线程编码
{
	/* start the application */
	sdk_run_config config;
	config.mem_trace_flag = SDK_RUN_DEBUG;
	ak_sdk_init(&config);
	
	vo_set_param();
	vi_set_param();
	venc_set_param();
	vdec_set_param();

	int ret = 0;
	ret = vo_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VO, "vo init failed!");
		return;
	}
	ret = vi_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VI, "vi init failed!");
		return;
	}
	ret = venc_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VENC, "venc init failed!");
		return;
	}
	ret = vdec_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VDEC, "vdec init failed!");
		return;
	}

	struct video_input_frame frame;
	enc_pair_set_source(&frame);
	venc_start(); 	//线程启动
	vdec_start();
	while (1)
	{
		ret = vi_get_one_frame(&frame, sizeof(frame));
		if (ret == SUCCESS)
		{
			// ak_print_normal(MODULE_ID_VI, "vi frame get successed!");
			vo_put_one_frame(frame.vi_frame.data);
			venc_thread_sem_post(); 	//通知编码线程启动

			vi_release_one_frame(&frame);
		}
		
	}
	vdec_close();
	venc_close();
	vi_close();
	vo_close();
}

void test2(void) 
{
	/* start the application */
	sdk_run_config config;
	config.mem_trace_flag = SDK_RUN_DEBUG;
	ak_sdk_init(&config);
	
	vo_set_param();
	vi_set_param();
	int ret = 0;
	ret = vo_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VO, "vo init failed!");
		return;
	}
	ret = vi_init();
	if (ret == FAILED)
	{
		ak_print_error_ex(MODULE_ID_VI, "vi init failed!");
		return;
	}
	// ret = vdec_inti();
	// if (ret == FAILED)
	// {
	// 	ak_print_error_ex(MODULE_ID_VDEC, "vdec init failed!");
	// 	return;
	// }
	// vdec_start();

	struct video_input_frame frame;
	while (1)
	{
		ret = vi_get_one_frame(&frame, sizeof(frame));
		if (ret == SUCCESS)
		{
			//ak_print_normal(MODULE_ID_VI, "vi frame get successed!");
			vo_put_one_frame(frame.vi_frame.data);
			vi_release_one_frame(&frame);
		}
		
	}
	// vdec_close();
	vi_close();
	vo_close();
}

void test4()
{
	sdk_run_config config;
	config.mem_trace_flag = SDK_RUN_DEBUG;
	ak_sdk_init(&config);
	
	vdec_set_param();
	ak_print_normal(MODULE_ID_VDEC, "vdec init");
	vdec_init();
	ak_print_normal(MODULE_ID_VDEC, "vdec start");
	vdec_start();
	while (1);
	vdec_close();

}


int main(int argc, char** argv)
{
	// test2(); //实时显示
	test3(); //多线程编码
	// test4();	//multipe threads decode
	// vdec_open_test();
}


//#include <stdio.h>
//
//int main(void)
//{
//	printf("hello world\n");
//	return 0;
//}

// /**
// * Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
// * File Name: ak_vi_demo.c
// * Description: This is a simple example to show how the VI module working.
// * Notes:
// * History: V1.0.0
// */
// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <getopt.h>
// #include <stdio.h>
// #include <errno.h>
// #include <signal.h>
// #include <ctype.h>
// #include <stdlib.h>
// #include <dirent.h>
// #include <sys/stat.h>

// #include "ak_common.h"
// #include "ak_common_graphics.h"
// #include "ak_vo.h"
// #include "ak_vi.h"
// #include "ak_mem.h"
// #include "ak_log.h"
// #include "ak_tde.h"

// //====================vi==================//
// #define LEN_HINT         512
// #define LEN_OPTION_SHORT 512

// #define DEF_FRAME_DEPTH		3
// #define RES_GROUP	5

// int frame_num = 10;
// int channel_num = 0;
// char* isp_path = "";
// char* save_path = "/mnt/";
// int main_res_id = 4;
// int sub_res_id = 0;

// //======================vo==================//
// #define MIPI_SC_WIDTH 1280
// #define MIPI_SC_HEIGHT 800

// #define RGB_SC_WIDTH  1024
// #define RGB_SC_HEIGHT  600

// #define LEN_HINT                512
// #define LEN_OPTION_SHORT        512
// #define MAX_DIS_NUM             4

// static char* pc_prog_name = NULL;                                                      //demo����
// static int screen_flag = 0;                                                         //mipi��Ļ
// static char* data_file = "/mnt/yuv";
// static char* logo_file = "/mnt/anyka.logo.577.160.rgb";
// static int display_num = 4;
// static AK_GP_FORMAT data_format = GP_FORMAT_YUV420P;
// static AK_GP_FORMAT out_format = GP_FORMAT_RGB888;


// /* support resolution list */
// RECTANGLE_S  res_group[RES_GROUP] =
// { {640, 360},			/* 640*360 */
//  {640, 480},			/*   VGA   */
//  {1280,720},			/*   720P  */
//  {960, 1080},			/*   1080i */
//  {1920,1080}			/*   1080P */
// };


// /*     ***********************************
// 	***********************************
// 	*
// 	use this demo
// 	must follow this:
// 	1. make sure the driver is insmode;
// 	2. mount the T card;
// 	3. the file path is exit;
// 	*
// 	***********************************
// 	***********************************
// */



// /**
//  * Preconditions:
//  * 1??TF card is already mounted
//  * 2??yuv_data is already created in /mnt
//  * 3??ircut is already opened at day mode
//  * 4??your main video progress must stop
//  */
// int main(int argc, char** argv)
// {
// 	/* start the application */
// 	sdk_run_config config;
// 	config.mem_trace_flag = SDK_RUN_DEBUG;
// 	ak_sdk_init(&config);

// 	ak_print_normal(MODULE_ID_VI, "*****************************************\n");
// 	ak_print_normal(MODULE_ID_VI, "** vi demo version: %s **\n", ak_vi_get_version());
// 	ak_print_normal(MODULE_ID_VI, "*****************************************\n");

// 	/*if (parse_option(argc, argv) == 0)
// 	{
// 		return 0;
// 	}*/

// 	//"eg: %s -n 1000 -c 1 -f isp_pr2000_dvp_pal_25fps_27M.conf -p vi_yuv/ -m 4 -s 0\n";
// 	//�ֶ�����vi����
// 	frame_num = 10000;
// 	channel_num = 0;
// 	isp_path = "/etc/isp_ar0230_dvp.conf";
// 	save_path = "/tmp/yuv/";
// 	main_res_id = 4;
// 	sub_res_id = 0;

// 	//"eg: %s -t 0 -f /mnt/yuv -n 1 -i 4 -o 1 -l /mnt/anyka.logo.577.160.rgb\n"
// 	//�ֶ�����vo����
// 	screen_flag = 1;
// 	data_file = "/tmp/yuv";
// 	display_num = 1;
// 	data_format = 5;
// 	out_format = 1;
// 	logo_file = "/mnt/anyka.logo.577.160.rgb";

// 	//==================================vi init start==========================//
// 		/*check param validate*/
// 	if (frame_num < 0 || channel_num < 0 || channel_num > 2 || strlen(isp_path) == 0 || strlen(save_path) == 0)
// 	{
// 		ak_print_error_ex(MODULE_ID_VI, "INPUT param error!\n");
// 		//help_hint(argv[0]);
// 		return 0;
// 	}

// 	/*check the data save path */
// 	/*if (check_dir(save_path) == 0)
// 	{
// 		ak_print_error_ex(MODULE_ID_VI, "save path is not existed!\n");
// 		return 0;
// 	}*/


// 	/*
// 	 * step 0: global value initialize
// 	 */
// 	int ret = -1;								//return value
// 	int width = res_group[main_res_id].width;
// 	int height = res_group[main_res_id].height;
// 	int subwidth = res_group[sub_res_id].width;
// 	int subheight = res_group[sub_res_id].height;

// 	/* open vi flow */

// 	/*
// 	 * step 1: open video input device
// 	 */
// 	ret = ak_vi_open(VIDEO_DEV0);
// 	if (AK_SUCCESS != ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device open failed\n");
// 		goto exit;
// 	}

// 	/*
// 	 * step 2: load isp config
// 	 */
// 	ret = ak_vi_load_sensor_cfg(VIDEO_DEV0, isp_path);
// 	if (AK_SUCCESS != ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device load isp cfg [%s] failed!\n", isp_path);
// 		goto exit;
// 	}

// 	/*
// 	 * step 3: get sensor support max resolution
// 	 */
// 	RECTANGLE_S res;				//max sensor resolution
// 	VI_DEV_ATTR	dev_attr;
// 	dev_attr.dev_id = VIDEO_DEV0;
// 	dev_attr.crop.left = 0;
// 	dev_attr.crop.top = 0;
// 	dev_attr.crop.width = width;
// 	dev_attr.crop.height = height;
// 	dev_attr.max_width = width;
// 	dev_attr.max_height = height;
// 	dev_attr.sub_max_width = subwidth;
// 	dev_attr.sub_max_height = subheight;

// 	/* get sensor resolution */
// 	ret = ak_vi_get_sensor_resolution(VIDEO_DEV0, &res);
// 	if (ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "Can't get dev[%d]resolution\n", VIDEO_DEV0);
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}
// 	else {
// 		ak_print_normal_ex(MODULE_ID_VI, "get dev res w:[%d]h:[%d]\n", res.width, res.height);
// 		dev_attr.crop.width = res.width;
// 		dev_attr.crop.height = res.height;
// 	}

// 	/*
// 	 * step 4: set vi device working parameters
// 	 * default parameters: 25fps, day mode
// 	 */
// 	ret = ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr);
// 	if (ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device set device attribute failed!\n");
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}

// 	/*
// 	 * step 5: set main channel attribute
// 	 */
// 	VI_CHN_ATTR chn_attr;
// 	chn_attr.chn_id = VIDEO_CHN0;
// 	chn_attr.res.width = width;
// 	chn_attr.res.height = height;
// 	chn_attr.frame_depth = DEF_FRAME_DEPTH;
// 	/*disable frame control*/
// 	chn_attr.frame_rate = 0;
// 	ret = ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr);
// 	if (ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN0);
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}
// 	ak_print_normal_ex(MODULE_ID_VI, "vi device main sub channel attribute\n");


// 	/*
// 	 * step 6: set sub channel attribute
// 	 */
// 	VI_CHN_ATTR chn_attr_sub;
// 	chn_attr_sub.chn_id = VIDEO_CHN1;
// 	chn_attr_sub.res.width = subwidth;
// 	chn_attr_sub.res.height = subheight;
// 	chn_attr_sub.frame_depth = DEF_FRAME_DEPTH;
// 	/*disable frame control*/
// 	chn_attr_sub.frame_rate = 0;
// 	ret = ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub);
// 	if (ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN1);
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}
// 	ak_print_normal_ex(MODULE_ID_VI, "vi device set sub channel attribute\n");

// 	/*
// 	 * step 7: enable vi device
// 	 */
// 	ret = ak_vi_enable_dev(VIDEO_DEV0);
// 	if (ret) {
// 		ak_print_error_ex(MODULE_ID_VI, "vi device enable device  failed!\n");
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}

// 	/*
// 	 * step 8: enable vi main channel
// 	 */
// 	ret = ak_vi_enable_chn(VIDEO_CHN0);
// 	if (ret)
// 	{
// 		ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable failed!\n", VIDEO_CHN0);
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}

// 	/*
// 	 * step 9: enable vi sub channel
// 	 */
// 	ret = ak_vi_enable_chn(VIDEO_CHN1);
// 	if (ret)
// 	{
// 		ak_print_error_ex(MODULE_ID_VI, "vi channel[%d] enable failed!\n", VIDEO_CHN1);
// 		ak_vi_close(VIDEO_DEV0);
// 		goto exit;
// 	}
// 	//===========================================vi init end==================================//

// 	//=========================================vo init start=================================//
// 	//print the version
// 	ak_print_normal(MODULE_ID_VI, "*****************************************\n");
// 	ak_print_normal(MODULE_ID_VI, "** vo version: %s **\n", ak_vo_get_version());
// 	ak_print_normal(MODULE_ID_VI, "*****************************************\n");

// 	/* parse the option */
// 	//if (parse_option(argc, argv) == AK_FALSE)            //���ͺ�����ѡ��
// 	//{
// 	//	return AK_FAILED;                                           //��ӡ�������˳�
// 	//}

// 	if (display_num < 0 || display_num > MAX_DIS_NUM)
// 	{
// 		ak_print_error_ex(MODULE_ID_VO, "display num is only support 0 to %d\n", MAX_DIS_NUM);
// 		return AK_FAILED;
// 	}

// 	/* store the file path */
// 	char file_path[64] = "";
// 	ret = 0;
// 	/* get the res and rotation */
// 	int dst_width = 0;
// 	int dst_height = 0;
// 	int rotate = 0;

// 	/* get the screen res */
// 	if (screen_flag == 1)
// 	{
// 		/* 1 means for RGB */
// 		dst_width = RGB_SC_WIDTH;
// 		dst_height = RGB_SC_HEIGHT;
// 		rotate = AK_GP_ROTATE_NONE;
// 	}
// 	else if (screen_flag == 0)
// 	{
// 		/* 0 means for MIPI */
// 		dst_width = MIPI_SC_WIDTH;
// 		dst_height = MIPI_SC_HEIGHT;
// 		rotate = AK_GP_ROTATE_90;
// 	}
// 	else
// 	{
// 		ak_print_error_ex(MODULE_ID_VO, "screen type is invalid\n");
// 		return AK_FAILED;
// 	}

// 	/* fill the struct to open the vo */
// 	struct ak_vo_param	param;

// 	/* fill the struct ready to open */
// 	param.width = dst_width;//1280;
// 	param.height = dst_height;// 800;
// 	param.format = out_format;  //format to output
// 	param.rotate = rotate;    //rotate value

// 	/* open vo */
// 	ret = ak_vo_open(&param, DEV_NUM);      //open vo

// 	if (ret != 0)
// 	{
// 		/* open failed return -1 */
// 		ak_print_error_ex(MODULE_ID_VO, "ak_vo_open failed![%d]\n", ret);
// 		return AK_FAILED;
// 	}

// 	/* create the video layer */
// 	struct ak_vo_layer_in video_layer;
// 	video_layer.create_layer.height = dst_height;   //800; //get the res
// 	video_layer.create_layer.width = dst_width;    //1280; //get the res
// 	/* just from (0,0) */
// 	video_layer.create_layer.left = 0;         //layer left pos
// 	video_layer.create_layer.top = 0;         // layer top pos
// 	video_layer.layer_opt = 0;
// 	video_layer.format = out_format;

// 	ret = ak_vo_create_video_layer(&video_layer, AK_VO_LAYER_VIDEO_1);
// 	if (ret != 0)
// 	{
// 		/* if failed, close the vo */
// 		ak_print_error_ex(MODULE_ID_VO, "ak_vo_create_video_layer failed![%d]\n", ret);
// 		ak_vo_close(DEV_NUM);       // if failed, close the vo
// 		return AK_FAILED;
// 	}

// 	/* create the gui layer */
// 	struct ak_vo_layer_in gui_layer;
// 	/* only support the given log file */
// 	gui_layer.create_layer.height = 160;    /* logo res */
// 	gui_layer.create_layer.width = 577;    /* logo res */
// 	gui_layer.create_layer.left = 0;       /* logo pos */
// 	gui_layer.create_layer.top = 0;       /* logo pos */
// 	gui_layer.format = out_format;  /* gui layer format */

// 	gui_layer.layer_opt = GP_OPT_COLORKEY; /* support the colorkey opt */

// 	gui_layer.colorkey.coloract = COLOR_DELETE;       /* delete the color */
// 	gui_layer.colorkey.color_min = 0xFFFF00;        /* min value */
// 	gui_layer.colorkey.color_max = 0xFFFFFF;        /* max value */

// 	struct ak_vo_layer_out gui_info;                /* output the gui layer info */
// 	ret = ak_vo_create_gui_layer(&gui_layer, AK_VO_LAYER_GUI_1, &gui_info);
// 	if (ret != 0)
// 	{
// 		ak_print_error_ex(MODULE_ID_VO, "ak_vo_create_gui_layer failed![%d]\n", ret);
// 		ak_vo_destroy_layer(AK_VO_LAYER_VIDEO_1);
// 		ak_vo_close(DEV_NUM);
// 		return AK_FAILED;
// 	}

// 	/* use the double buff mode */
// 	ak_vo_set_fbuffer_mode(AK_VO_BUFF_DOUBLE);

// 	/* support the yuv 420p */
// 	int len = (1920 * 1080 * 3) / 2;

// 	/* get the size of the pixel */
// 	switch (data_format)
// 	{
// 		/* 565 */
// 	case GP_FORMAT_RGB565:
// 	case GP_FORMAT_BGR565:
// 		len = 1920 * 1080 * 2;
// 		break;
// 		/* 888 */
// 	case GP_FORMAT_BGR888:
// 	case GP_FORMAT_RGB888:
// 		len = 1920 * 1080 * 3;
// 		break;
// 		/* 8888 */
// 	case GP_FORMAT_ABGR8888:
// 	case GP_FORMAT_ARGB8888:
// 	case GP_FORMAT_RGBA8888:
// 	case GP_FORMAT_BGRA8888:
// 		len = 1920 * 1080 * 4;
// 		break;
// 		/* YUV */
// 	case GP_FORMAT_YUV420P:
// 	case GP_FORMAT_YUV420SP:
// 		break;
// 	default:
// 		ak_print_error_ex(MODULE_ID_VO, "here not support the tileyuv data\n");

// 	}

// 	// void* data = ak_mem_dma_alloc(MODULE_ID_VO, len);
// 	// if (data == NULL)
// 	// {
// 	// 	ak_print_error_ex(MODULE_ID_VO, "Can't malloc DMA memory!\n");
// 	// 	ret = AK_FAILED;
// 	// 	goto err;
// 	// }

// 	/* 1-4 partion screen to show the picture */
// 	int dou = 1;
// 	if (display_num == 1)
// 		dou = 1;
// 	else
// 		dou = 2;

// 	/* add logo to gui layer */
// 	//ret = add_logo_to_gui();
// 	//if (ret != 0)
// 	//{
// 	//	ak_print_error_ex(MODULE_ID_VO, "add_logo_to_gui failed![%d]\n", ret);
// 	//	goto err;
// 	//}
// 	//========================================vo init end==================================//
// 		/*
// 		 * step 10: start to capture and save yuv frames
// 		 */
// 	int count = 0;
// 	struct video_input_frame frame;

// 	ak_print_normal(MODULE_ID_VI, "capture start\n");

// 	/*
// 	 * To get frame by loop
// 	 */
// 	while (count < frame_num) {
// 		memset(&frame, 0x00, sizeof(frame));

// 		/* to get frame according to the channel number */
// 		int ret = ak_vi_get_frame(channel_num, &frame);

// 		if (!ret) {
// 			/*
// 			 * Here, you can implement your code to use this frame.
// 			 * Notice, do not occupy it too long.
// 			 */
// 			ak_print_normal_ex(MODULE_ID_VI, "[%d] main chn yuv len: %u\n", count,
// 				frame.vi_frame.len);
// 			ak_print_normal_ex(MODULE_ID_VI, "[%d] main chn phyaddr: %lu\n", count,
// 				frame.phyaddr);
// 			//display(frame.vi_frame.data, frame.vi_frame.len);//�Լ�д����ʾ����

// 			//输出
// 			//��ʾͼ��
// 			int ret = 0;
// 			// memset(data, 0, len);
// 			//ret = fread(data, 1, len, fp);      // read the file
// 			// ak_print_normal(MODULE_ID_VO, "read [%d] byte to dma buffer\n", ret);

// 			/* obj add */
// 			struct ak_vo_obj obj;

// 			/* set obj src info*/
// 			obj.format = data_format;
// 			obj.cmd = GP_OPT_SCALE;
// 			obj.vo_layer.width = 1920;
// 			obj.vo_layer.height = 1080;
// 			obj.vo_layer.clip_pos.top = 0;
// 			obj.vo_layer.clip_pos.left = 0;
// 			obj.vo_layer.clip_pos.width = 1920;
// 			obj.vo_layer.clip_pos.height = 1080;

// 			ak_mem_dma_vaddr2paddr(frame.vi_frame.data, &(obj.vo_layer.dma_addr));

// 			/* show as the screen partion set */
// 			int counter = display_num;
// 			if (display_num == 1)
// 			{
// 				/* set dst_layer 1 info*/
// 				obj.dst_layer.top = 0;
// 				obj.dst_layer.left = 0;
// 				obj.dst_layer.width = dst_width;
// 				obj.dst_layer.height = dst_height;
// 				/* display obj 1*/
// 				ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
// 			}
// 			else
// 			{
// 				/* more than one partion */
// 				if (counter)
// 				{
// 					/* set dst_layer 1 info*/
// 					obj.dst_layer.top = 0;
// 					obj.dst_layer.left = 0;
// 					obj.dst_layer.width = dst_width / dou;
// 					obj.dst_layer.height = dst_height / dou;
// 					/* display obj 1*/
// 					ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
// 					counter--;
// 				}

// 				if (counter != 0)
// 				{
// 					/* set dst_layer 2 info*/
// 					obj.dst_layer.top = 0;
// 					obj.dst_layer.left = dst_width / dou;
// 					obj.dst_layer.width = dst_width / dou;
// 					obj.dst_layer.height = dst_height / dou;//400;
// 					/* display obj 1*/
// 					ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
// 					counter--;
// 				}

// 				if (counter)
// 				{
// 					/* set dst_layer 3 info*/
// 					obj.dst_layer.top = dst_height / dou;//400;
// 					obj.dst_layer.left = 0;
// 					obj.dst_layer.width = dst_width / dou;
// 					obj.dst_layer.height = dst_height / dou;//400;
// 					/* display obj 1*/
// 					ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
// 					counter--;
// 				}

// 				if (counter)
// 				{
// 					/* set dst_layer 4 info*/
// 					obj.dst_layer.top = dst_height / dou;//400;
// 					obj.dst_layer.left = dst_width / dou;
// 					obj.dst_layer.width = dst_width / dou;
// 					obj.dst_layer.height = dst_height / dou;//400;
// 					/* display obj 1*/
// 					ak_vo_add_obj(&obj, AK_VO_LAYER_VIDEO_1);
// 					counter--;
// 				}

// 			}

// 			/* flush to screen */
// 			int cmd = AK_VO_REFRESH_VIDEO_GROUP & 0x100;
// 			// cmd |= AK_VO_REFRESH_GUI_GROUP & 0x10000;
// 			ak_vo_refresh_screen(cmd);


// 			//if (channel_num == VIDEO_CHN0)
// 			//	save_yuv_data(save_path, count, &frame, attr);
// 			//else
// 			//	save_yuv_data(save_path, count, &frame, attr_sub);

// 			/*
// 			 * in this context, this frame was useless,
// 			 * release frame data
// 			 */
// 			ak_vi_release_frame(channel_num, &frame);
// 			count++;
// 		}
// 		else {

// 			/*
// 			 *	If getting too fast, it will have no data,
// 			 *	just take breath.
// 			 */
// 			ak_print_normal_ex(MODULE_ID_VI, "get frmae failed!\n");
// 			ak_sleep_ms(10);
// 		}
// 	}

// 	ak_print_normal(MODULE_ID_VI, "capture finish\n\n");


// err:
// 	// if (data)
// 	// 	ak_mem_dma_free(data);

// 	ak_vo_destroy_layer(AK_VO_LAYER_VIDEO_1);
// 	ak_vo_destroy_layer(AK_VO_LAYER_GUI_1);
// 	/* close the vo */
// 	ak_vo_close(DEV_NUM);

// 	/*
// 	 * step 11: release resource
// 	 */
// 	ak_vi_disable_chn(VIDEO_CHN0);
// 	ak_vi_disable_chn(VIDEO_CHN1);
// 	ak_vi_disable_dev(VIDEO_DEV0);
// 	ret = ak_vi_close(VIDEO_DEV0);

// exit:
// 	/* exit */
// 	ak_print_normal(MODULE_ID_VI, "exit vi demo\n");
// 	return ret;
// }
