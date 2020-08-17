#include "xmu_ai.h"
#include "xmu_common.h"

Ai::Ai(/* args */)
{
    // BaseThread(50*1024, -1);
    set_param();
    init();
    dbf = new DataBuffer(MODULE_ID_MEMORY, aenc_mem);
    // dbf(MODULE_ID_AENC, aenc_mem);
}

Ai::~Ai()
{
    delete dbf;
}

void Ai::set_param()
{
    ai_handle_id = 0;
    aenc_handle_id = 0;
    send_frame_end = 0;

    sample_rate = AK_AUDIO_SAMPLE_RATE_8000; //"[NUM]  采样率[8000 16000 32000 44100 48000]" 
    channel_num = AUDIO_CHANNEL_MONO;  //"[NUM]  音量[1 2]" 
    enc_type = AK_AUDIO_TYPE_MP3;   //"[TYPE]    编码类型[mp3 aac amr g711a g711u pcm]"
    // save_time = 20000;        // set save time(ms)
    // save_path= "/mnt/";    // set save path
}

int Ai::init()
{
    // create_record_file(enc_type, sample_rate, channel_num);

    enum ak_ai_source  source = AI_SOURCE_MIC;

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
        return AK_FAILED;
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
        return AK_FAILED;
    }
    ak_print_normal(MODULE_ID_AENC,"ak_aenc_open OK\n");
    if (aenc_param.type == AK_AUDIO_TYPE_AAC) 
    {
        struct aenc_attr attr;
        attr.aac_head = AENC_AAC_SAVE_FRAME_HEAD;
        ak_aenc_set_attr(aenc_handle_id, &attr); 
    }

    ak_ai_start_capture(ai_handle_id);
}

int Ai::get_frame_len(int encode_type, int sample_rate, int channel)
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

void Ai::run()
{
    struct frame ai_frame = {0};
    struct audio_stream stream = {0};
    // unsigned long long start_ts = 0;// use to save capture start time
    // unsigned long long end_ts = 0;    // the last frame time

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
                // fwrite(stream.data, stream.len, 1, fp);
                ret = dbf->rb_write(stream.data, stream.len);
                // if (ret == AK_FAILED)
                //     ak_print_error_ex(MODULE_ID_ADEC, "write failed\n");
                print_playing_dot();
            }
            else
            {
                ak_ai_release_frame(ai_handle_id, &ai_frame);
                continue;
            }
        }
        ak_aenc_release_stream(aenc_handle_id, &stream);
        ak_ai_release_frame(ai_handle_id, &ai_frame);
        ak_sleep_ms(10);
    }
}

void Ai::print_playing_dot(void)
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
        ak_print_normal(MODULE_ID_AENC, ".\n");
    }
}

static void * callback(void *arg)
{
    ((Ai*)arg)->run();
}

void Ai::start(void)
{
    BaseThread::start(callback);
}