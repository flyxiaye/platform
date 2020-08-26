#include "xmu_ao.h"

#define RECORD_READ_LEN         4096 /* read audio file buffer length */
// #define LEN_HINT                512
// #define LEN_OPTION_SHORT        512

Ao::Ao(/* args */)
{
    set_param();
    init();
}

Ao::~Ao()
{
    ak_ao_set_speaker(ao_handle_id, AUDIO_FUNC_DISABLE);
    /* ao close */
    ak_ao_close(ao_handle_id);
}

void Ao::set_param()
{
    ao_handle_id = -1;
    // send_frame_end = 0;

    sample_rate = AK_AUDIO_SAMPLE_RATE_8000;
    channel_num = AUDIO_CHANNEL_MONO;
    // play_file = "/mnt/ak_ao_test.pcm";
    // dec_type = AK_AUDIO_TYPE_PCM;
}

int Ao::init()
{
    /* open audio output */
    struct ak_audio_out_param param;
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    param.pcm_data_attr.channel_num = channel_num;
    param.pcm_data_attr.sample_rate = sample_rate;
    param.dev_id = 0;

    /* ao open */
    if(ak_ao_open(&param, &ao_handle_id)) 
    {
        ak_print_error_ex(MODULE_ID_ADEC,"\t ak_ao_open failed !\n");
        return AK_FAILED;
    }

    ak_ao_set_speaker(ao_handle_id, AUDIO_FUNC_ENABLE);
    ak_ao_set_gain(ao_handle_id, 7);// set_gain
    // ak_ao_set_eq_attr()
    // ak_ao_set_volume(ao_handle_id, 1 << 30);

    ak_ao_clear_frame_buffer(ao_handle_id);// clear_frame_buffer
}

void Ao::print_playing_dot(void)
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