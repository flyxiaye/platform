#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>

#include "ak_common.h"

#include "ak_ao.h"
#include "ak_adec.h"
#include "ak_thread.h"
#include "ak_log.h"

#ifdef AK_RTOS
#include "rtthread.h"
#endif

#define RECORD_READ_LEN         4096 /* read audio file buffer length */
#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

FILE *fp = NULL;

int ao_handle_id = -1;
int send_frame_end = 0;

int sample_rate = 8000;
int channel_num = 1;
char *play_file = "/mnt/ak_ao_test.pcm";
int dec_type = AK_AUDIO_TYPE_PCM;

/*     ***********************************
    ***********************************
    *
    use this demo
    must follow this:
    1. make sure the driver is insmode;
    2. mount the T card;
    3. the file path is exit;
    *
    ***********************************
    ***********************************
*/

char ac_option_hint[  ][ LEN_HINT ] = {
    "       打印帮助信息" ,
    "[NUM]  采样率[8000 12000 11025 16000 22050 24000 32000 44100 48000]" ,
    "[NUM]  通道数[1 2]" ,
    "[TYPE] 编码类型[mp3 aac amr g711a g711u pcm]" ,
    "[FILE] 播放的文件名称" ,
};

struct option option_long[ ] = {
    { "help"        , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "sample-rate" , required_argument , NULL , 's' } , //"[NUM]  采样率[8000 16000 32000 44100 48000]" ,
    { "channel-num" , required_argument , NULL , 'c' } , //"[NUM]  通道数[1 2]" ,
    { "decode_type" , required_argument , NULL , 'd' } , //"[TYPE] 编码类型[mp3 aac amr g711a g711u pcm]" ,
    { "play-file"   , required_argument , NULL , 'p' } , //"[FILE] 播放的文件名称" ,
    {0, 0, 0, 0}
};

/**
 * open_record_file - open_record_file
 * notes:
 */
static FILE* open_record_file(const char *record_file)
{
    FILE *fp = fopen(record_file, "r");
    if(NULL == fp) 
    {
        ak_print_error_ex(MODULE_ID_ADEC, "open %s failed\n", record_file);
        return NULL;
    }

    ak_print_normal(MODULE_ID_ADEC, "open record file: %s OK\n", record_file);
    return fp;
}

/**
 * print_playing_dot - print . when playing every second
 * notes:
 */
static void print_playing_dot(void)
{
    static unsigned char first_flag = 1;
    static struct ak_timeval cur_time;
    static struct ak_timeval print_time;

    if (first_flag) 
    {
        first_flag = 0;
        ak_get_ostime(&cur_time);
        ak_get_ostime(&print_time);
        ak_print_normal_ex(MODULE_ID_ADEC,"\n.");
    }

    /* get time */
    ak_get_ostime(&cur_time);
    if (ak_diff_ms_time(&cur_time, &print_time) >= 1000) 
    {
        ak_get_ostime(&print_time);
        ak_print_normal_ex(MODULE_ID_ADEC, ".");
    }
}

/**
 * read_record_file - read_record_file
 * notes:
 */
static void* read_record_file(void *arg)
{
    unsigned int read_len = 0;
    unsigned int total_len = 0;
    unsigned char data[RECORD_READ_LEN] = {0};

    int adec_handle_id = (*((int *)arg));
    ak_print_normal_ex(MODULE_ID_ADEC, "\n\t thread create success \n");

    while (1)
    {
        /* read the record file stream */
        memset(data, 0x00, RECORD_READ_LEN);
        read_len = fread(data, sizeof(char), RECORD_READ_LEN, fp);
        if(read_len > 0) 
        {
            total_len += read_len;
            /* play roop */
            ak_adec_send_stream(adec_handle_id, data, read_len, 1);
            print_playing_dot();            
        } 
        else if(0 == read_len) 
        {
            /* read to the end of file */
            send_frame_end = 1;
            ak_print_normal(MODULE_ID_ADEC, "\n\tread to the end of file\n");
            break;
        } 
        else 
        {
            ak_print_error(MODULE_ID_ADEC, "\nread, %s\n", strerror(errno));
            break;
        }
        ak_sleep_ms(10);
    }
    /* read over the file, must call ak_adec_read_end */
    ak_adec_send_stream_end(adec_handle_id);
    ak_print_notice_ex(MODULE_ID_ADEC, "%s exit\n", __func__);

    ak_thread_exit();
    return NULL;
}

/**
 * get_frame_then_play - 
 * notes:
 */
static void* get_frame_then_play(void *arg)
{
    int adec_handle_id = (*((int *)arg));
    
    struct frame pcm_frame = {0};
    int play_len = 0;

    while(1)
    {    
        if (ak_adec_get_frame(adec_handle_id, &pcm_frame, 1))
        {
            ak_print_error_ex(MODULE_ID_ADEC, "can not get frame=%p,len=%d\n", pcm_frame.data, pcm_frame.len);
            ak_sleep_ms(10);
            break;
        }

        if (pcm_frame.len == 0 || pcm_frame.data == NULL)
        {
            ak_sleep_ms(20);
            continue;
        }

        if (ak_ao_send_frame(ao_handle_id, pcm_frame.data, pcm_frame.len, &play_len))
        {
            ak_adec_release_frame(adec_handle_id, &pcm_frame);
            break;
        }
        ak_adec_release_frame(adec_handle_id, &pcm_frame);
    }
    ak_ao_wait_play_finish(ao_handle_id);

    ak_print_notice_ex(MODULE_ID_ADEC, "%s exit\n", __func__);
    ak_thread_exit();
    return NULL;
}

/**
 * parse_adec_type - 
 * notes:
 */
static int parse_adec_type(const char *adec_type)
{
    int ret = -1;
    char type[10] = {0};
    int type_len = strlen(adec_type);

    /* the type length <= 5,such as g711u */
    if (type_len <= 5) 
    {
        sprintf(type, "%s", adec_type);

        int i = 0;
        for (i=0; i<type_len; ++i) 
        {
            type[i] = tolower(type[i]);
        }

        ak_print_notice_ex(MODULE_ID_ADEC, "audio decode type: %s\n", type);

        /* check the audio file type */
        if (0 == strcmp(type, "mp3"))
            ret = AK_AUDIO_TYPE_MP3;
        else if (0 == strcmp(type, "amr"))
            ret = AK_AUDIO_TYPE_AMR;
        else if (0 == strcmp(type, "aac"))
            ret = AK_AUDIO_TYPE_AAC;
        else if (0 == strcmp(type, "g711a"))
            ret = AK_AUDIO_TYPE_PCM_ALAW;
        else if (0 == strcmp(type, "g711u"))
            ret = AK_AUDIO_TYPE_PCM_ULAW;
        else if (0 == strcmp(type, "pcm")) 
            ret = AK_AUDIO_TYPE_PCM;
        else if (0 == strcmp(type, "dra")) /* ret = AK_AUDIO_TYPE_DRA; */
            ret = AK_AUDIO_TYPE_UNKNOWN;
        else /* unknow type */
            ret = AK_AUDIO_TYPE_UNKNOWN;
        
    }
    return ret;
}

/**
 * help_hint - 
 * notes:
 */
static int help_hint( char *pc_prog_name )
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
    }
    printf("\n\n");
    return 0;
}

/**
 * get_option_short - 
 * notes:
 */
char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        c_option = p_option[ i ].val;
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

/**
 * parse_option - 
 * notes:
 */
int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    
    char *type = "pcm";

    if (argc < 6 && argc != 1) 
    {
        help_hint( argv[ 0 ] );
        c_flag = 0;
        goto parse_option_end;
    }

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) 
    {
        switch(i_option) 
        {
        case 'h' :                                                          //help
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        case 's' :                                                          //sample-rate
            sample_rate = atoi( optarg );
            break;
        case 'c' :                                                          //volume
            channel_num = atoi( optarg );
            printf("----channel_num- %d-----\n", channel_num);
            break;
        case 'd' :                                                          //volume
            type = optarg;
            dec_type = parse_adec_type(type);
            printf("----dec_type- %d-----\n", dec_type);
            break;
        case 'p' :                                                          //pcm-file
            play_file = optarg;
            break;
        }
    }
    
parse_option_end:
    return c_flag;
}

/**
 * Preconditions:
 * 1. T card is already mounted
 * 2. speaker must ready
 */
#ifdef AK_RTOS
int adec_sample_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config;
    config.mem_trace_flag = SDK_RUN_DEBUG;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_ADEC, "*****************************************\n");
	ak_print_normal(MODULE_ID_ADEC, "** adec demo version: %s **\n", ak_adec_get_version());
    ak_print_normal(MODULE_ID_ADEC, "*****************************************\n");
	
    if( parse_option( argc, argv ) == 0 ) 
    {
		ak_print_error_ex(MODULE_ID_ADEC,"parse param invalid!\n");
		return 0;
    }

    /* open audio output */
    struct ak_audio_out_param param;
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    param.pcm_data_attr.channel_num = channel_num;
    param.pcm_data_attr.sample_rate = sample_rate;
    param.dev_id = 0;

    /* open audio file */

    fp = open_record_file(play_file);
    if(NULL == fp) 
    {
        return -1;
    }

    /* ao open */
    if(ak_ao_open(&param, &ao_handle_id)) 
    {
        ak_print_error_ex(MODULE_ID_ADEC,"\t ak_ao_open failed !\n");
        goto fp_end;
    }

    ak_ao_set_speaker(ao_handle_id, AUDIO_FUNC_ENABLE);
    ak_ao_set_gain(ao_handle_id, 4);// set_gain

    ak_ao_clear_frame_buffer(ao_handle_id);// clear_frame_buffer

    /* open audio encode */
    struct adec_param adec_param;
    adec_param.type = dec_type;
    adec_param.adec_data_attr.sample_bits = 16;
    adec_param.adec_data_attr.channel_num = channel_num;
    adec_param.adec_data_attr.sample_rate = param.pcm_data_attr.sample_rate;

    /* open adec */
    int adec_handle_id = -1;
    if (ak_adec_open(&adec_param, &adec_handle_id))// open
    {
        goto ao_end;
    }

    /* amr file,have to skip the head 6 byte */
    unsigned char data[10] = {0};
    if (AK_AUDIO_TYPE_AMR == dec_type) 
    {
        fread(data, 1, 6, fp);
    }

    ak_pthread_t send_stream_tid;
    ak_pthread_t play_tid;
    // thread
    int ret = ak_thread_create(&(send_stream_tid), read_record_file,
                            &adec_handle_id, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    
    if (ret)
    {
        goto adec_end;
    }

    ret = ak_thread_create(&(play_tid), get_frame_then_play,
                            &adec_handle_id, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    
    if (ret)
    {
        goto adec_end;
    }

    ak_sleep_ms(100);
    // join
    ak_thread_join(send_stream_tid);
    ak_thread_join(play_tid);

adec_end:
    /* adec close */
    if(-1 != adec_handle_id) 
    {
        ak_adec_close(adec_handle_id);
        adec_handle_id = -1;
    }
ao_end:
    if(ao_handle_id != -1) 
    {
        /* ao disable speaker */
        ak_ao_set_speaker(ao_handle_id, AUDIO_FUNC_DISABLE);
        /* ao close */
        ak_ao_close(ao_handle_id);
        ao_handle_id = -1;
    }

fp_end:
    if(NULL != fp) 
    {
        fclose(fp);
        fp = NULL;
    }
    
    ak_print_normal_ex(MODULE_ID_ADEC,"----- %s exit -----\n", argv[0]);

    return 0;
}

#ifdef AK_RTOS
MSH_CMD_EXPORT(adec_sample_main, Audio Decode Sample Program);
#endif

/* end of file */
