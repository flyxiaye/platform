#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>


#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_thread.h"
#include "ak_log.h"

#ifdef AK_RTOS
#include "rtthread.h"
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512


FILE *fp;            /* save file handle */

int ai_handle_id = 0;
int aenc_handle_id = NULL;
int send_frame_end = 0;

int sample_rate = 8000;
int channel_num = 1;
int enc_type = AK_AUDIO_TYPE_MP3;
int save_time = 20000;        // set save time(ms)
char *save_path= "/mnt/";    // set save path

/*     
    ***********************************
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
    "[SEC]  采集时间" ,
    "[NUM]  采样率[8000 12000 11025 16000 22050 24000 32000 44100 48000]" ,
    "[NUM]  通道数[1,2]" ,
    "[TYPE] 编码类型[mp3 aac amr g711a g711u pcm]" ,
    "[PATH] 文件保存路径" ,
};

struct option option_long[ ] = {
    { "help"        , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "second"      , required_argument , NULL , 't' } , //"[SEC]  采集时间" ,
    { "sample-rate" , required_argument , NULL , 's' } , //"[NUM]  采样率[8000 16000 32000 44100 48000]" ,
    { "channel"     , required_argument , NULL , 'c' } , //"[NUM]  音量[1 2]" ,
    { "eccode_type" , required_argument , NULL , 'e' } , //"[TYPE]    编码类型[mp3 aac amr g711a g711u pcm]" ,
    { "path"        , required_argument , NULL , 'p' } , //"[PATH] 文件保存路径" ,
    {0, 0, 0, 0}
};

/**  
 * print_playing_dot - 
 * 
 * notes: 
 */
static void print_playing_dot(void)
{
    static unsigned char first_flag = 1;
    static struct ak_timeval cur_time;
    static struct ak_timeval print_time;

    if (first_flag) {
        first_flag = 0;
        ak_get_ostime(&cur_time);
        ak_get_ostime(&print_time);
        ak_print_normal(MODULE_ID_AENC, "\n.");
    }

    ak_get_ostime(&cur_time);
    if (ak_diff_ms_time(&cur_time, &print_time) >= 1000) {
        ak_get_ostime(&print_time);
        ak_print_normal(MODULE_ID_AENC, ".");
    }
}

/**  
 * create_record_file - create the audio record file
 * @enc_type[IN]: encode type [mp3/amr/aac/g711a/g711u/pcm]
 * notes: 
 */
static int create_record_file(int enc_type, int sample_rate, int channel_num)
{
    char time_str[20] = {0};
    char file_path[255] = {0};
    struct ak_date date;

    /* get the current time */
    ak_get_localdate(&date);
    ak_date_to_string(&date, time_str);

    /* set the file full path */
    switch(enc_type) 
    {
    case AK_AUDIO_TYPE_AAC:
        sprintf(file_path, "%s%s_%d_%d.aac", save_path, time_str, sample_rate, channel_num);
        break;
    case AK_AUDIO_TYPE_AMR:
        sprintf(file_path, "%s%s_%d_%d.amr", save_path, time_str, sample_rate, channel_num);
        break;
    case AK_AUDIO_TYPE_MP3:
        sprintf(file_path, "%s%s_%d_%d.mp3", save_path, time_str, sample_rate, channel_num);
        break;
    case AK_AUDIO_TYPE_PCM_ALAW:
        sprintf(file_path, "%s%s_%d_%d.g711a", save_path, time_str, sample_rate, channel_num);
        break;
    case AK_AUDIO_TYPE_PCM_ULAW:
        sprintf(file_path, "%s%s_%d_%d.g711u", save_path, time_str, sample_rate, channel_num);
        break;
    case AK_AUDIO_TYPE_PCM:
        sprintf(file_path, "%s%s_%d_%d.pcm", save_path, time_str, sample_rate, channel_num);
        break;
    default:
        sprintf(file_path, "%s%s_%d_%d", save_path, time_str, sample_rate, channel_num);
        break;
    }

    fp = fopen(file_path, "w+");
    if(NULL == fp) 
    {
        ak_print_error_ex(MODULE_ID_AENC, "create file err\n");
        return -1;
    }
    
    /* write AMR file head */
    if (AK_AUDIO_TYPE_AMR == enc_type) 
    {
        const unsigned char amrheader[]= "#!AMR\n";
        fwrite(amrheader, sizeof(amrheader) - 1, 1, fp);
    }
    ak_print_normal_ex(MODULE_ID_AENC, "create amr file: %s OK\n", file_path);    
    return 0;
}

/**  
 * parse_aenc_type - 
 * 
 * notes: 
 */
static int parse_aenc_type(const char *aenc_type)
{
    int ret = -1;
    char type[10] = {0};
    int type_len = strlen(aenc_type);
    
    /* the type length <= 5,such as g711u */
    if (type_len <= 5) 
    {
        sprintf(type, "%s", aenc_type);

        int i = 0;
        for (i=0; i<type_len; ++i) 
        {
            type[i] = tolower(type[i]);
        }
        ak_print_notice_ex(MODULE_ID_AENC, "audio encode type: %s\n", type);
        
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
        else
            ret = AK_AUDIO_TYPE_UNKNOWN;        
        
    }
    return ret;    
}

static int get_frame_len(int encode_type, int sample_rate, int channel)
{
    int frame_len = 960;
    if (AK_AUDIO_TYPE_MP3 == encode_type || AK_AUDIO_TYPE_AAC == encode_type)
    {
        switch (sample_rate)
        {
        case AK_AUDIO_SAMPLE_RATE_8000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 : 1920;//60
            break;
        case AK_AUDIO_SAMPLE_RATE_11025:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 882 : 1764;//40
            break;
        case AK_AUDIO_SAMPLE_RATE_12000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 : 1920;//40
            break;
        case AK_AUDIO_SAMPLE_RATE_16000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 : 1920;//30
            break;
        case AK_AUDIO_SAMPLE_RATE_22050:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 882 : 1764;//20
            break;
        case AK_AUDIO_SAMPLE_RATE_24000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 : 1920;//20
            break;
        case AK_AUDIO_SAMPLE_RATE_32000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 1920 : 3840;//30
            break;
        case AK_AUDIO_SAMPLE_RATE_44100:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 1764 : 3528;//20
            break;
        case AK_AUDIO_SAMPLE_RATE_48000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 1920 : 3840;//20
            break;
        default:
            break;
        }
    }
    else
    {
        switch (sample_rate)
        {
        case AK_AUDIO_SAMPLE_RATE_8000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1920;
            break;
        case AK_AUDIO_SAMPLE_RATE_11025:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1600;
            break;
        case AK_AUDIO_SAMPLE_RATE_12000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1600;
            break;
        case AK_AUDIO_SAMPLE_RATE_16000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1600;
            break;
        case AK_AUDIO_SAMPLE_RATE_22050:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1600;
            break;
        case AK_AUDIO_SAMPLE_RATE_24000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 960 :1600;
            break;
        case AK_AUDIO_SAMPLE_RATE_32000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 1600 :3200;
            break;
        case AK_AUDIO_SAMPLE_RATE_44100:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 3200 :4000;
            break;
        case AK_AUDIO_SAMPLE_RATE_48000:
            frame_len = (channel == AUDIO_CHANNEL_MONO) ? 3200 :6400;
            break;
        default:
            break;
        }

    }

    ak_print_notice_ex(MODULE_ID_AENC, "frame_len=%d\n", frame_len);

    return frame_len;
}

/**  
 * get_frame_encode - 
 * 
 * notes: 
 */
static void* get_frame_encode(void *arg)
{
    struct frame ai_frame = {0};
    struct audio_stream stream = {0};
    unsigned long long start_ts = 0;// use to save capture start time
    unsigned long long end_ts = 0;    // the last frame time

    int ret = -1;

    while(1)
    {        
        ret = ak_ai_get_frame(ai_handle_id, &ai_frame, 0);
        if (ret) 
        {
            if (ERROR_AI_NO_DATA == ret)
            {
                ak_sleep_ms(10);
                continue;
            }
            else 
            {
                ak_print_error_ex(MODULE_ID_AENC, "ak_ai_get_frame error ret=%d\n", ret);
                break;
            }
        }
        ret = ak_aenc_send_frame(aenc_handle_id, &ai_frame, 0);
        if (ret)
        {
            ak_ai_release_frame(ai_handle_id, &ai_frame);
            ak_print_error_ex(MODULE_ID_AENC, "send frame error\n");
            break;
        }

        ret = ak_aenc_get_stream(aenc_handle_id, &stream, 0);
        if (ret)
        {
            if (ERROR_AENC_NO_DATA == ret)
            {
                ak_sleep_ms(10);
                ak_ai_release_frame(ai_handle_id, &ai_frame);
                continue;
            }
            else 
            {
                ak_ai_release_frame(ai_handle_id, &ai_frame);
                ak_print_error_ex(MODULE_ID_AENC, "ak_aenc_get_stream error ret=%d\n", ret);
                break;
            }
        }
        else
        {    
            if (stream.data && stream.len)
            {
                fwrite(stream.data, stream.len, 1, fp);
                print_playing_dot();
            }
            else
            {
                ak_ai_release_frame(ai_handle_id, &ai_frame);
                continue;
            }
        }

        if (0 == start_ts)
        {
            start_ts = stream.ts;
        }

        end_ts = stream.ts;

        ak_aenc_release_stream(aenc_handle_id, &stream);
        ak_ai_release_frame(ai_handle_id, &ai_frame);

        /* time is up */
        if ((end_ts - start_ts) >= save_time) {
            ak_print_error_ex(MODULE_ID_AENC, "time is up\n");
            break;
        }
        ak_sleep_ms(10);
    }

    ak_print_notice_ex(MODULE_ID_AENC, "%s exit\n", __func__);
    ak_thread_exit();
    return NULL;
}

/**  
 * help_hint - 
 * 
 * notes: 
 */
static int help_hint( char *pc_prog_name )
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) 
    {
        printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
    }
    printf("\n\n");
    return 0;
}

/**  
 * get_option_short - 
 * 
 * notes: 
 */
char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) 
    {
        c_option = p_option[ i ].val;
        switch( p_option[ i ].has_arg )
        {
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
 * 
 * notes: 
 */
int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    char *type= "mp3";    // set save path

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
        case 't' :                                                          //second
            save_time = atoi( optarg ) * 1000 ;
            break;
        case 's' :                                                          //sample-rate
            sample_rate = atoi( optarg );
            break;
        case 'c' :                                                          //volume
            channel_num = atoi( optarg );
            printf("----channel_num- %d-----\n", channel_num);
            break;
        case 'e' :                                                          //volume
            type = optarg;
            printf("----enc_type- %s-----\n", type);
            enc_type = parse_aenc_type(type);
            printf("---nnn  -enc_type- %d-----\n", enc_type);
            break;
        case 'p' :                                                          //path
            save_path = optarg;
            break;
        default:
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;        
        }
    }
parse_option_end:
    return c_flag;
}

/**
 * Preconditions:
 * 1. T card is already mounted
 * 2. mic or linein must ready
 */
#ifdef AK_RTOS
int aenc_sample_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config;
    config.mem_trace_flag = SDK_RUN_DEBUG;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_AENC, "*****************************************\n");
	ak_print_normal(MODULE_ID_AENC, "** aenc demo version: %s **\n", ak_aenc_get_version());
    ak_print_normal(MODULE_ID_AENC, "*****************************************\n");
	
    if( parse_option( argc, argv ) == 0 ) 
    {
		ak_print_error_ex(MODULE_ID_AENC,"parse param invalid!\n");
		return 0;
    }

    create_record_file(enc_type, sample_rate, channel_num);

    int source = AI_SOURCE_MIC;

    ak_print_normal(MODULE_ID_AENC,"version: %s\n\n", ak_aenc_get_version());

    int volume = 5;

    /* open audio input */
    struct ak_audio_in_param ai_param;
    
    ai_param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    ai_param.pcm_data_attr.channel_num = channel_num;
    ai_param.pcm_data_attr.sample_rate = sample_rate;
    ai_param.dev_id = 0;

    if (ak_ai_open(&ai_param, &ai_handle_id)) 
    {
        goto fp_end;
    }
    ak_print_normal(MODULE_ID_AENC,"\t 1. ak_ai_open OK\n");
    ak_ai_enable_nr(ai_handle_id, AUDIO_FUNC_ENABLE);// enable_nr
    ak_ai_enable_agc(ai_handle_id, AUDIO_FUNC_ENABLE);// enable_agc
    
    ak_ai_set_gain(ai_handle_id, volume);// enable_agc

    if (ak_ai_set_source(ai_handle_id, source))// enable_agc
        ak_print_error_ex(MODULE_ID_AENC, "set ai source %d fail\n", source);
    else
        ak_print_normal_ex(MODULE_ID_AENC, "set ai source %d success\n", source);
    
    ak_ai_clear_frame_buffer(ai_handle_id);
    int frame_len = get_frame_len(enc_type, sample_rate, channel_num);
    ak_ai_set_frame_length(ai_handle_id, frame_len);

    /* open audio encode */
    struct aenc_param aenc_param;
    aenc_param.type = enc_type;
    aenc_param.aenc_data_attr.sample_bits = ai_param.pcm_data_attr.sample_bits;
    aenc_param.aenc_data_attr.channel_num = ai_param.pcm_data_attr.channel_num;
    aenc_param.aenc_data_attr.sample_rate = ai_param.pcm_data_attr.sample_rate;
    
    if (ak_aenc_open(&aenc_param, &aenc_handle_id)) 
    {
        
        ak_print_error_ex(MODULE_ID_AENC, "ak_aenc_open failed\n");
        goto ai_end;
    }
    ak_print_normal(MODULE_ID_AENC,"ak_aenc_open OK\n");
    if (aenc_param.type == AK_AUDIO_TYPE_AAC) 
    {
        struct aenc_attr attr;
        attr.aac_head = AENC_AAC_SAVE_FRAME_HEAD;
        ak_aenc_set_attr(aenc_handle_id, &attr); 
    }

    ak_ai_start_capture(ai_handle_id);
    
    ak_pthread_t encode_tid;

    int ret = ak_thread_create(&(encode_tid), get_frame_encode, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    
    if (ret)
    {
        goto aenc_end;
    }
    
    ak_thread_join(encode_tid);

aenc_end:

    if (-1 != aenc_handle_id) {
        /* aenc close */
        ak_aenc_close(aenc_handle_id);
        aenc_handle_id = -1;
    }

ai_end:
    if (-1 != ai_handle_id) {
        ak_ai_stop_capture(ai_handle_id);
        /* ai close */
        ak_ai_close(ai_handle_id);
        ai_handle_id = -1;
    }

fp_end:

    if (NULL != fp) {
        fclose(fp);
        fp = NULL;
    }

    printf("----- %s exit -----\n", argv[0]);

    return 0;
}

#ifdef AK_RTOS
MSH_CMD_EXPORT(aenc_sample_main, Audio encode Sample Program);
#endif

/* end of file */
