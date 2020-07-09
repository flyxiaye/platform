#define LEN_HINT                    512
#define LEN_PATH                    1024
#define LEN_DATA                    256
#define LEN_OPTION_SHORT            512

#define DEFAULT_WIDTH               1024
#define DEFAULT_HEIGHT              600
#define DEFAULT_POS_TOP             0
#define DEFAULT_POS_LEFT            0
#define DEFAULT_POS_WIDTH           1024
#define DEFAULT_POS_HEIGHT          600
#define DEFAULT_FORMAT              TDE_FORMAT_RGB888
#define DEFAULT_FILE_SRC            "./ak_tde_s_test.rgb"
#define DEFAULT_FILE_BG             "./ak_tde_bg_test.rgb"
#define DEFAULT_DIR_YUV             "./yuv"
#define DEFAULT_NUM_BUFFYUV         10

#define LEN_OFFSET_V                4                                           //yuv��uֵƫ���� y=0 u=h*w v=h*w+h*w/4
#define YUV_TEST_PRINT              100
#define DEV_GUI                     "/dev/fb0"                                  //gui lcdʹ�õ��豸����
#define SIZE_ERROR                  0xffffffff
#define USEC2SEC                    1000000

#define BITS_PER_PIXEL              24
#define LEN_COLOR                   8
#define DUAL_FB_FIX                 fb_fix_screeninfo_param.reserved[ 0 ]
#define DUAL_FB_VAR                 fb_var_screeninfo_param.reserved[ 0 ]

#ifndef AK_SUCCESS
#define AK_SUCCESS 0
#endif

#ifndef AK_FAILED
#define AK_FAILED -1
#endif

enum color_offset {
	OFFSET_RED = 16,
	OFFSET_GREEN = 8,
	OFFSET_BLUE = 0,
};

struct regloop {
	regex_t regex_t_use ;
	char *pc_pattern;
	int i_flags;
	int i_offset ;
} ;

int main( int argc, char **argv );
int test_yuv( char *pc_pach );
int test_tde( void );

/*
	help_hint: ����option_long��ac_option_hint�����ӡ������Ϣ
	return: 0
*/
static int help_hint(void);

/*
	get_option_short: ����option_long����ѡ���ַ���
	@p_option[IN]: struct option�����ַ
	@i_num_option[IN]: ����Ԫ�ظ���
	@pc_option_short[IN]: ���������ַ
	@i_len_option[IN]: ���鳤��
	return: pc_option_short
*/
char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option );

/*
	parse_option: �Դ����ѡ����н���
	@argc[IN]: �ӳ�����ڴ����ѡ������
	@argv[IN]: ѡ�����ݵ��ַ���ָ������
	return: AK_TRUE:��������  AK_FALSE:�˳�Ӧ��,�ڴ�ӡ��������ѡ����ʹ����ʱ��ʹ��
*/
int parse_option( int argc, char **argv );

/*
	get_file_size: ��ȡ�ļ�����
	return: �ɹ�:AK_SUCCESS ʧ��:AK_FAILED
*/
unsigned int get_file_size( char *pc_filename );

/*
	ak_gui_open: ��lcd��Ļ�豸
	@p_fb_fix_screeninfo[OUT]:������Ļ��ֻ����Ϣ
	@p_fb_var_screeninfo[OUT]:������Ļ�Ŀ�������Ϣ
	@pv_addr[OUT]:�Դ�ĵ�ַ
	return: �ɹ�:AK_SUCCESS ʧ��:AK_FAILED
*/
int ak_gui_open( struct fb_fix_screeninfo *p_fb_fix_screeninfo, struct fb_var_screeninfo *p_fb_var_screeninfo, void **pv_addr );

/*
	ak_gui_close: �ر�lcd��Ļ�豸
	return: �ɹ�:AK_SUCCESS ʧ��:AK_FAILED
*/
int ak_gui_close( void );

/*
	lcd_show_fix: ��ӡlcd��Ļ��ֻ��������Ϣ
	@fix[IN]:struct fb_fix_screeninfoָ��
	return: NULL
*/
void lcd_show_fix(struct fb_fix_screeninfo *fix);

/*
	lcd_show_var: ��ӡlcd��Ļ�Ŀ��޸ĵĲ�����Ϣ
	@var[IN]:struct fb_var_screeninfoָ��
	return: NULL
*/
void lcd_show_var(struct fb_var_screeninfo *var);

/*
	dump_file: ���Դ澵�����ݱ�����ļ�
	@pc_file[IN]:�ļ�����
	@pv_addr[IN]:�Դ��ַ
	@i_size[IN]:����
	return: д���ļ�����
*/
int dump_file( char *pc_file, void *pv_addr, int i_size );

/*
	init_regloop: ��ʼ��һ��������ʽ��ѭ����ȡ�ṹ��
	@p_regloop[IN]:ѭ���ṹ��
	@pc_pattern[IN]:������ʽ
	@i_flags[IN]:��־
	return: �ɹ�:AK_SUCCESS ʧ��:AK_FAILED
*/
int init_regloop( struct regloop *p_regloop , char *pc_pattern , int i_flags );

/*
	free_regloop: �ͷ�һ��������ʽ��ѭ����ȡ�ṹ�������ָ��
	@p_regloop[IN]:ѭ���ṹ��
	return: 0
*/
int free_regloop( struct regloop *p_regloop );

/*
	reset_regloop: ����һ������ѭ���ṹ��
	@p_regloop[IN]:ѭ���ṹ��
	return: 0
*/
int reset_regloop( struct regloop *p_regloop );

/*
	match_regloop: ����һ������ѭ���ṹ��
	@p_regloop[IN]:ѭ���ṹ��
	@pc_buff[IN]:������ַ���
	@pc_res[OUT]:ÿ�λ�ȡ������ƥ����
	@i_len_res[IN]:ƥ����ָ��Ļ���������
	return: ����ƥ��������
*/
int match_regloop( struct regloop *p_regloop , char *pc_buff , char *pc_res , int i_len_res );
