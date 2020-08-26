#include "adec.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "xmu_common.h"

#define RECORD_READ_LEN 4096

Adec::Adec(Ao *ao)
{
    this->ao = ao;
    set_param();
    init();
}

Adec::~Adec()
{
    ak_adec_close(adec_handle_id);
}

void Adec::set_param()
{
    adec_handle_id = -1;
    dec_type = AK_AUDIO_TYPE_AAC;
}

int Adec::init()
{
    /* open audio encode */
    struct adec_param adec_param;
    adec_param.type = dec_type;
    adec_param.adec_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    adec_param.adec_data_attr.channel_num = ao->channel_num;
    adec_param.adec_data_attr.sample_rate = ao->sample_rate;

    /* open adec */
    // int adec_handle_id = -1;
    if (ak_adec_open(&adec_param, &adec_handle_id))// open
    {
        return AK_FAILED;
    }
    return AK_SUCCESS;
}

void Adec::run()
{   
    struct frame pcm_frame = {0};
    int play_len = 0;

    while(1)
    {    
        if (ak_adec_get_frame(adec_handle_id, &pcm_frame, 1))
        {
            // ak_print_error_ex(MODULE_ID_ADEC, "can not get frame=%p,len=%d\n", pcm_frame.data, pcm_frame.len);
            ak_sleep_ms(10);
            // break;
            continue;
        }

        if (pcm_frame.len == 0 || pcm_frame.data == NULL)
        {
            ak_sleep_ms(20);
            continue;
        }

        if (ak_ao_send_frame(ao->ao_handle_id, pcm_frame.data, pcm_frame.len, &play_len))
        {
            ak_adec_release_frame(adec_handle_id, &pcm_frame);
            break;
        }
        ak_adec_release_frame(adec_handle_id, &pcm_frame);
    }
    ak_ao_wait_play_finish(ao->ao_handle_id);
}

static void *callback(void *arg)
{
    ((Adec*)arg)->run();
}

void Adec::start()
{
    BaseThread::start(callback);
}

AdecSend::AdecSend(Adec *ad)
{
    adec = ad;
    adec_handle_id = ad->adec_handle_id;
    send_frame_end = 0;
    dbf = new DataBuffer(MODULE_ID_ADEC, adec_mem);
}

AdecSend::~AdecSend()
{
    delete dbf;
}

void AdecSend::run()
{
    int read_len = 0;
    // unsigned int total_len = 0;
    unsigned char data[RECORD_READ_LEN];

    ak_print_normal_ex(MODULE_ID_ADEC, "\n\t thread create success \n");
    // ak_adec_send_stream_end(adec_handle_id);
    while (1)
    {
        dbf->rb_read(data, RECORD_READ_LEN, &read_len);
        if(read_len > 0) 
        {
            ak_adec_send_stream(adec_handle_id, data, read_len, 1);
        } 
        ak_sleep_ms(10);
    }
    ak_adec_send_stream_end(adec_handle_id);
}

static void *callback2(void *arg)
{
    ((AdecSend*)arg)->run();
}

void AdecSend::start()
{
    BaseThread::start(callback2);
}