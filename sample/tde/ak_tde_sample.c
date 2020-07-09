/**
* Copyright (C) 2019 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_tde_demo.c
* Description: This is a simple example to show how the tde module working.
* Notes:
* History: V1.0.0
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ak_tde_sample.h"
#include "ak_common.h"
#include "ak_mem.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "ak_mem.h"
#include "ak_log.h"

static int fd_gui = 0;

char *pc_prog_name = NULL;                                                      //demo名称
char *pc_file_src = DEFAULT_FILE_SRC;                                           //tde_opt使用的测试图片
char *pc_file_bg = DEFAULT_FILE_BG;                                             //tde_opt使用的背景图片

struct ak_tde_cmd tde_cmd_param;                                                //进行tde操作的结构体
struct fb_fix_screeninfo fb_fix_screeninfo_param;                               //屏幕固定参数
struct fb_var_screeninfo fb_var_screeninfo_param;                               //屏幕可配置参数
char c_view_screen = AK_FALSE;                                                  //是否查看屏幕参数
char c_reset_screen = AK_FALSE;                                                 //是否重置屏幕参数
void *p_vaddr_fb = NULL;                                                        //fb0的虚拟内存地址
char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "打印帮助信息" ,
    "显示屏幕参数信息" ,
    "拷贝图层" ,
    "拉伸图层" ,
    "重置屏幕参数" ,
    "[HEX] 使用指定颜色进行矩形填充" ,
    "[0-3] 选择指定角度 (0:DEGREE0 1:DEGREE90 2:DEGREE180 3:DEGREE270)" ,
    "[0-15] 透明度设置" ,
    "\"[[HEX-COLOR-MIN] [HEX-COLOR-MAX] [0:DEL 1:KEEP]\" 颜色过滤设置" ,
    "[FILE] 源图文件   (DEFAULT: ak_tde_s_test.rgb)" ,
    "[FILE] 背景图文件 (DEFAULT: ak_tde_bg_test.rgb)" ,
    "\"[PIC-WIDTH] [PIC-HEIGHT] [RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 源图坐标参数" ,
    "\"[PIC-WIDTH] [PIC-HEIGHT] [RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 背景图坐标参数" ,
    "[NUM] 背景图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP 6:ARGB8888 7:RGBA8888 8:ABGR8888 9:BGRA8888 10:TILED32X4" ,
    "[NUM] 目标图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP" ,
    "[NUM] 源图图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP 6:ARGB8888 7:RGBA8888 8:ABGR8888 9:BGRA8888 10:TILED32X4" ,
    "\"[RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 源图贴图的坐标参数" ,
    "" ,
};

double af_format_byte[  ] = { 2 , 3 , 2 , 3 , 1.5 , 1.5 , 4 , 4 , 4 , 4 , 0 };  //图形格式占用的字节数
struct ak_tde_layer tde_layer_src = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , GP_FORMAT_RGB888 } ;    //源坐标
struct ak_tde_layer tde_layer_tgt = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , GP_FORMAT_RGB888 } ;    //目标坐标
struct ak_tde_layer tde_layer_bg = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , GP_FORMAT_RGB888 } ;     //背景图坐标
struct ak_tde_layer tde_layer_screen = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , GP_FORMAT_RGB888 } ; //屏幕坐标

struct option option_long[ ] = {
    { "help"              , no_argument       , NULL , 'h' } ,                  //"打印帮助信息" ,
    { "show-screen"       , no_argument       , NULL , 'A' } ,                  //"显示屏幕参数信息" ,
    { "opt-blit"          , no_argument       , NULL , 'B' } ,                  //"拷贝图层" ,
    { "opt-scale"         , no_argument       , NULL , 'C' } ,                  //"拉伸图层" ,
    { "reset-screen"      , no_argument       , NULL , 'D' } ,                  //"重置屏幕参数" ,
    { "opt-fillrect"      , required_argument , NULL , 'a' } ,                  //"[HEX] 使用指定颜色进行矩形填充" ,
    { "opt-rotate"        , required_argument , NULL , 'b' } ,                  //"[0-3] 选择指定角度 (0:DEGREE0 1:DEGREE90 2:DEGREE180 3:DEGREE270)" ,
    { "opt-transparent"   , required_argument , NULL , 'c' } ,                  //"[0-15] 透明度设置" ,
    { "opt-colorkey"      , required_argument , NULL , 'd' } ,                  //"\"[[HEX-COLOR-MIN] [HEX-COLOR-MAX] [0:DEL 1:KEEP]\" 颜色过滤设置" ,
    { "file-s"            , required_argument , NULL , 'e' } ,                  //"[FILE] 源图文件   (DEFAULT: ak_tde_s_test.rgb)" ,
    { "file-bg"           , required_argument , NULL , 'f' } ,                  //"[FILE] 背景图文件 (DEFAULT: ak_tde_bg_test.rgb)" ,
    { "rect-s"            , required_argument , NULL , 'g' } ,                  //"\"[PIC-WIDTH] [PIC-HEIGHT] [RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 源图坐标参数" ,
    { "rect-bg"           , required_argument , NULL , 'i' } ,                  //"\"[PIC-WIDTH] [PIC-HEIGHT] [RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 背景图坐标参数" ,
    { "format-bg"         , required_argument , NULL , 'j' } ,                  //"[NUM] 背景图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP 6:ARGB8888 7:RGBA8888 8:ABGR8888 9:BGRA8888 10:TILED32X4" ,
    { "format-out"        , required_argument , NULL , 'k' } ,                  //"[NUM] 目标图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP" ,
    { "format-s"          , required_argument , NULL , 'l' } ,                  //"[NUM] 源图图层格式 0:RGB565 1:RGB888 2:BGR565 3:BGR888 4:YUV420P 5:YUV420SP 6:ARGB8888 7:RGBA8888 8:ABGR8888 9:BGRA8888 10:TILED32X4" ,
    { "rect-t"            , required_argument , NULL , 'm' } ,                  //"\"[RECT-TOP] [RECT-LEFT] [RECT-WIDTH] [RECT-HEIGHT]\" 源图贴图的坐标参数" ,
    { 0                   , 0                 , 0    ,  0  } ,                  //"" ,
 };

/*
    help_hint: 根据option_long和ac_option_hint数组打印帮助信息
    return: 0
*/
static int help_hint(void)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
        }
    }
    printf("\n\n");
    return 0;
}

/*
    get_option_short: 根据option_long填充短选项字符串
    @p_option[IN]: struct option数组地址
    @i_num_option[IN]: 数组元素个数
    @pc_option_short[IN]: 填充的数组地址
    @i_len_option[IN]: 数组长度
    return: pc_option_short
*/
char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
            case no_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
                break;
            case required_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
                break;
            case optional_argument:
                i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
                break;
        }
    }
    return pc_option_short;
}

/*
    parse_option: 对传入的选项进行解释
    @argc[IN]: 从程序入口传入的选项数量
    @argv[IN]: 选项内容的字符串指针数组
    return: AK_TRUE:继续运行  AK_FALSE:退出应用,在打印帮助或者选项解释错误的时候使用
*/
int parse_option( int argc, char **argv )
{
    int i_option, i_times;
    struct regloop regloop_num;
    char ac_data[ LEN_DATA ];
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = AK_TRUE;
    pc_prog_name = argv[ 0 ];

    memset( &tde_cmd_param , 0 , sizeof( struct ak_tde_cmd ) );

    init_regloop( &regloop_num, "[0-9A-Fa-f]+", REG_EXTENDED | REG_NEWLINE );   //初始化一个匹配16进制数字正则表达式
    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
            case 'h' :                                                          //help
                help_hint();
                c_flag = AK_FALSE;
                goto parse_option_end;
            case 'A' :                                                          //show-screen 显示lcd 参数
                c_view_screen = AK_TRUE;
                break;
            case 'B' :                                                          //opt-blit 图层拷贝操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_BLIT ;
                break;
            case 'C' :                                                          //opt-scale 拉伸操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_SCALE ;
                break;
            case 'D' :                                                          //reset-screen 重置屏幕参数
                c_reset_screen = AK_TRUE;
                break;
            case 'a' :                                                          //opt-fillrect 画矩形操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_FILLRECT;
                sscanf( optarg , "%x" , &tde_cmd_param.color_rect );
                break;
            case 'b' :                                                          //opt-rotate 旋转操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_ROTATE ;
                tde_cmd_param.rotate_param = atoi(optarg);
                break;
            case 'c' :                                                          //opt-transparent 透明度操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_TRANSPARENT ;
                tde_cmd_param.alpha = atoi(optarg);
                break;
            case 'd' :                                                          //opt-colorkey 颜色过滤(colorkey)操作
                tde_cmd_param.opt = tde_cmd_param.opt | GP_OPT_COLORKEY ;
                reset_regloop( &regloop_num );
                i_times = 0;
                while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){         //寻找匹配数字
                    switch( i_times ) {
                        case 0:
                            sscanf( ac_data , "%x" , &tde_cmd_param.colorkey_param.color_min );     //颜色值min
                            break;
                        case 1:
                            sscanf( ac_data , "%x" , &tde_cmd_param.colorkey_param.color_max );     //颜色值max
                            break;
                        case 2:
                            tde_cmd_param.colorkey_param.coloract = atoi( ac_data ) ;     //对范围内颜色是保留还是删除
                            break;
                    }
                    i_times ++;
                }
                break;
            case 'e' :                                                          //file-s 源图文件
                pc_file_src = optarg;
                break;
            case 'f' :                                                          //file-bg 源图文件
                pc_file_bg = optarg;
                break;
            case 'g' :                                                          //rect-s 源图层宽高坐标信息
                reset_regloop( &regloop_num );
                i_times = 0;
                while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){         //寻找匹配数字
                    switch( i_times ) {
                        case 0:
                            tde_layer_src.width = atoi( ac_data ) ;             //图层宽度
                            break;
                        case 1:
                            tde_layer_src.height = atoi( ac_data ) ;            //图层高度
                            break;
                        case 2:
                            tde_layer_src.pos_left = atoi( ac_data ) ;          //选中区域坐标X值
                            break;
                        case 3:
                            tde_layer_src.pos_top = atoi( ac_data ) ;           //选中区域坐标Y值
                            break;
                        case 4:
                            tde_layer_src.pos_width = atoi( ac_data ) ;         //选中区域宽度
                            break;
                        case 5:
                            tde_layer_src.pos_height = atoi( ac_data ) ;        //选中区域高度
                            break;
                    }
                    i_times ++;
                }
                break;
            case 'i' :                                                          //rect-bg 背景图层参数设置
                reset_regloop( &regloop_num );
                i_times = 0;
                while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){    //寻找匹配数字
                    switch( i_times ) {
                        case 0:
                            tde_layer_bg.width = atoi( ac_data ) ;              //图层宽度
                            break;
                        case 1:
                            tde_layer_bg.height = atoi( ac_data ) ;             //图层高度
                            break;
                        case 2:
                            tde_layer_bg.pos_left = atoi( ac_data ) ;          //选中区域坐标X值
                            break;
                        case 3:
                            tde_layer_bg.pos_top = atoi( ac_data ) ;           //选中区域坐标Y值
                            break;
                        case 4:
                            tde_layer_bg.pos_width = atoi( ac_data ) ;         //选中区域宽度
                            break;
                        case 5:
                            tde_layer_bg.pos_height = atoi( ac_data ) ;        //选中区域高度
                            break;
                    }
                    i_times ++;
                }
                break;
            case 'j' :
                tde_layer_bg.format_param = atoi(optarg);
                break;
            case 'k' :
                tde_layer_screen.format_param = atoi(optarg);
                break;
            case 'l' :
                tde_layer_src.format_param = atoi(optarg);
                break;
            case 'm' :                                                          //rect-t 源图贴图的坐标信息
                reset_regloop( &regloop_num );
                i_times = 0;
                while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){    //寻找匹配数字
                    switch( i_times ) {
                        case 0:
                            tde_layer_tgt.pos_left = atoi( ac_data ) ;          //选中区域坐标X值
                            break;
                        case 1:
                            tde_layer_tgt.pos_top = atoi( ac_data ) ;           //选中区域坐标Y值
                            break;
                        case 2:
                            tde_layer_tgt.pos_width = atoi( ac_data ) ;         //选中区域宽度
                            break;
                        case 3:
                            tde_layer_tgt.pos_height = atoi( ac_data ) ;        //选中区域高度
                            break;
                    }
                    i_times ++;
                }
                break;
            default :
                help_hint();
                c_flag = AK_FALSE;
                goto parse_option_end;
        }
    }
parse_option_end:
    free_regloop( &regloop_num );                                               //释放正则表达式结构的分配资源
    return c_flag;
}

/*
    ak_gui_open: 打开lcd屏幕设备
    @p_fb_fix_screeninfo[OUT]:返回屏幕的只读信息
    @p_fb_var_screeninfo[OUT]:返回屏幕的可配置信息
    @pv_addr[OUT]:显存的地址
    return: 成功:AK_SUCCESS 失败:AK_FAILED
*/
int ak_gui_open( struct fb_fix_screeninfo *p_fb_fix_screeninfo, struct fb_var_screeninfo *p_fb_var_screeninfo, void **pv_addr )
{
    if ( fd_gui > 0 ) {
        return AK_SUCCESS;
    }
    else if ( ( fd_gui = open( DEV_GUI, O_RDWR ) ) > 0 ) {

        if (ioctl(fd_gui, FBIOGET_FSCREENINFO, p_fb_fix_screeninfo) < 0) {
            close( fd_gui );
            return AK_FAILED;
        }
        if (ioctl(fd_gui, FBIOGET_VSCREENINFO, p_fb_var_screeninfo) < 0) {
            close( fd_gui );
            return AK_FAILED;
        }
        *pv_addr = ( void * )mmap( 0 , p_fb_fix_screeninfo->smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_gui, 0 );

        if( c_reset_screen == AK_TRUE ) {
            p_fb_var_screeninfo->xres = p_fb_var_screeninfo->xres_virtual;
            p_fb_var_screeninfo->yres = p_fb_var_screeninfo->yres_virtual;
            p_fb_var_screeninfo->bits_per_pixel = BITS_PER_PIXEL;
            p_fb_var_screeninfo->red.offset = OFFSET_RED;
            p_fb_var_screeninfo->red.length = LEN_COLOR ;
            p_fb_var_screeninfo->green.offset = OFFSET_GREEN;
            p_fb_var_screeninfo->green.length = LEN_COLOR ;
            p_fb_var_screeninfo->blue.offset = OFFSET_BLUE;
            p_fb_var_screeninfo->blue.length = LEN_COLOR ;
            if (ioctl(fd_gui, FBIOPUT_VSCREENINFO, p_fb_var_screeninfo) < 0) {
                close( fd_gui );
                return AK_FAILED;
            }
            memset( *pv_addr, 0 , p_fb_fix_screeninfo->smem_len );
        }
        tde_layer_screen.width      = fb_var_screeninfo_param.xres;
        tde_layer_screen.height     = fb_var_screeninfo_param.yres;
        tde_layer_screen.pos_left   = 0;
        tde_layer_screen.pos_top    = 0;
        tde_layer_screen.pos_width  = fb_var_screeninfo_param.xres;
        tde_layer_screen.pos_height = fb_var_screeninfo_param.yres;
        tde_layer_screen.phyaddr    = fb_fix_screeninfo_param.smem_start;
        return AK_SUCCESS;
    }
    else {
        return AK_FAILED;
    }
}

/*
    ak_gui_close: 关闭lcd屏幕设备
    return: 成功:AK_SUCCESS 失败:AK_FAILED
*/
int ak_gui_close( void )
{
    if ( fd_gui > 0 ){
        if( p_vaddr_fb != NULL ) {
            munmap( p_vaddr_fb, fb_fix_screeninfo_param.smem_len );
        }
        if( close( fd_gui ) == 0 ) {
            fd_gui = 0 ;
            return AK_SUCCESS;
        }
        else {

            return AK_FAILED;
        }
    }
    else {
        return AK_SUCCESS;
    }
}

/*
    get_file_size: 获取文件长度
    return: 成功:AK_SUCCESS 失败:AK_FAILED
*/
unsigned int get_file_size( char *pc_filename )
{
    struct stat stat_buf;
    if( stat( pc_filename , &stat_buf ) < 0 ){
        return AK_FAILED ;
    }
    return ( unsigned int )stat_buf.st_size ;
}

int main( int argc, char **argv )
{
    sdk_run_config config= {0};

    config.mem_trace_flag = SDK_RUN_DEBUG;
    ak_sdk_init( &config );
    ak_print_normal(MODULE_ID_TDE, "*****************************************\n");
	ak_print_normal(MODULE_ID_TDE, "** tde demo version: %s **\n", ak_tde_get_version());
    ak_print_normal(MODULE_ID_TDE, "*****************************************\n");
    if( parse_option( argc, argv ) == AK_FALSE ) {                              //解释和配置选项
        return 0;                                                               //打印帮助后退出
    }
    ak_tde_open( );
    ak_gui_open( &fb_fix_screeninfo_param, &fb_var_screeninfo_param, &p_vaddr_fb);

    test_tde( );                                                                //tde图形测试

    ak_gui_close( );
    ak_tde_close( );
    return 0;
}

int test_tde( void )                                                            //tde图形操作测试
{
    unsigned int i_filesize_src, i_filesize_bg;
    FILE *pFILE;
    void *p_vaddr_src= NULL, *p_vaddr_bg= NULL;
    int i_dmasize_src = tde_layer_src.width * tde_layer_src.height * af_format_byte[ tde_layer_src.format_param ];      //源图片的设定大小
    int i_dmasize_bg  = tde_layer_bg.width * tde_layer_bg.height * af_format_byte[ tde_layer_bg.format_param ];         //背景图片的设定大小

    if ( ( i_dmasize_src == 0 ) || ( i_dmasize_bg == 0 ) ) {
        printf( "FORMAT ERROR. i_dmasize_src= %d i_dmasize_bg= %d\n", i_dmasize_src, i_dmasize_bg );
        goto test_tde_end;
    }

    if ( ( DUAL_FB_FIX == AK_TRUE ) && ( DUAL_FB_VAR != 0 ) ) {                 //如果使用双buffer的话，将buffer设置为使用第1个buffer
        DUAL_FB_VAR = 0;
        ioctl( fd_gui, FBIOPUT_VSCREENINFO, &fb_var_screeninfo_param ) ;
    }

    i_filesize_src = get_file_size( pc_file_src );                              //获取文件长度
    i_filesize_bg = get_file_size( pc_file_bg );

    if ( ( i_filesize_src != i_dmasize_src ) || ( i_filesize_bg != i_dmasize_bg ) ) {
        printf( "FILE SIZE NOT FIT FORMAT. i_filesize_src= %d i_dmasize_src= %d i_filesize_bg= %d i_dmasize_bg= %d\n",
                i_filesize_src, i_filesize_bg, i_filesize_bg, i_dmasize_bg );
        goto test_tde_end;
    }

    p_vaddr_src = ak_mem_dma_alloc( 1, i_dmasize_src );                         //分配pmem内存
    p_vaddr_bg = ak_mem_dma_alloc( 1, i_dmasize_bg );
    ak_mem_dma_vaddr2paddr( p_vaddr_bg , ( unsigned long * )&tde_layer_bg.phyaddr );      //获取背景图片dma物理地址
    ak_mem_dma_vaddr2paddr( p_vaddr_src , ( unsigned long * )&tde_layer_src.phyaddr );    //获取源图片dma物理地址


    if( ( i_filesize_src != SIZE_ERROR ) && ( i_filesize_src != 0 ) ) {         //源图片
        pFILE = fopen( pc_file_src , "rb" );                                    //将图片内容读入pmem
        fseek(pFILE, 0, SEEK_SET);
        fread( ( char * )p_vaddr_src, 1, i_filesize_src, pFILE);
        fclose( pFILE );
    }
    if( ( i_filesize_bg != SIZE_ERROR ) && ( i_filesize_bg != 0 ) ) {           //背景图片
        pFILE = fopen( pc_file_bg , "rb" );                                     //将图片内容读入pmem
        fseek( pFILE, 0, SEEK_SET ) ;
        fread( ( char * )p_vaddr_bg, 1, i_filesize_bg, pFILE);
        fclose( pFILE );
        ak_tde_opt_scale( &tde_layer_bg, &tde_layer_screen );                   //贴背景图片
    }
    else {
        memset( p_vaddr_fb , 0xff , fb_fix_screeninfo_param.smem_len );
    }


    tde_layer_tgt.width = fb_var_screeninfo_param.xres;                         //屏幕宽
    tde_layer_tgt.height = fb_var_screeninfo_param.yres;                        //屏幕高
    tde_layer_tgt.phyaddr = fb_fix_screeninfo_param.smem_start;                 //屏幕的fb物理地址直接赋值
    tde_cmd_param.tde_layer_src = tde_layer_src;
    tde_cmd_param.tde_layer_dst = tde_layer_tgt;
    ak_tde_opt( &tde_cmd_param );                                               //贴源图片

test_tde_end:
    if( p_vaddr_src != NULL ) {
        ak_mem_dma_free( p_vaddr_src );
    }
    if( p_vaddr_bg != NULL ) {
        ak_mem_dma_free( p_vaddr_bg );
    }
    return 0;
}

/*
    init_regloop: 初始化一个正则表达式的循环读取结构体
    @p_regloop[IN]:循环结构体
    @pc_pattern[IN]:正则表达式
    @i_flags[IN]:标志
    return: 成功:AK_SUCCESS 失败:AK_FAILED
*/
int init_regloop( struct regloop *p_regloop , char *pc_pattern , int i_flags )
{
    int i_status = 0;

    memset( p_regloop , 0 , sizeof( struct regloop ) );

    if ( ( pc_pattern != NULL ) &&
         ( ( i_status = regcomp( &p_regloop->regex_t_use , pc_pattern , i_flags ) ) == REG_NOERROR ) ) {
        p_regloop->pc_pattern = pc_pattern;
    }
    else {
        return AK_FALSE;
    }
    return AK_TRUE;
}

/*
    free_regloop: 释放一个正则表达式的循环读取结构体的正则指针
    @p_regloop[IN]:循环结构体
    return: 0
*/
int free_regloop( struct regloop *p_regloop )
{
    if( p_regloop->pc_pattern != NULL ) {
        regfree( &p_regloop->regex_t_use );
    }
    return 0;
}

/*
    reset_regloop: 重置一个正则循环结构体
    @p_regloop[IN]:循环结构体
    return: 0
*/
int reset_regloop( struct regloop *p_regloop )
{
    if( p_regloop != NULL ) {
        p_regloop->i_offset = 0;
    }
    return 0;
}

/*
    match_regloop: 重置一个正则循环结构体
    @p_regloop[IN]:循环结构体
    @pc_buff[IN]:输入的字符串
    @pc_res[OUT]:每次获取的正则匹配结果
    @i_len_res[IN]:匹配结果指针的缓冲区长度
    return: 正则匹配结果长度
*/
int match_regloop( struct regloop *p_regloop , char *pc_buff , char *pc_res , int i_len_res )
{
    int i_len_match = 0, i_len_cpy = 0;
    regmatch_t regmatch_t_res ;
    char *pc_now = pc_buff + p_regloop->i_offset ;

    pc_res[ 0 ] = 0;
    if ( regexec( &p_regloop->regex_t_use , pc_now , 1 , &regmatch_t_res , 0 ) == 0 ) {
        if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {                 //匹配到的字符串长度为0
            return 0;
        }

        if ( i_len_res > i_len_match ) {
            i_len_cpy = i_len_match;
        }
        else {
            i_len_cpy = i_len_res - 1;
        }
        memcpy( pc_res , pc_now + regmatch_t_res.rm_so , i_len_cpy ) ;
        pc_res[ i_len_cpy ] = 0 ;
        p_regloop->i_offset += regmatch_t_res.rm_eo;
    }
    return i_len_cpy ;
}
