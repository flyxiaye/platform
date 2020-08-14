#ifndef XMU_AO_H
#define XMU_AO_H

extern "C" {
#include "ak_common.h"
#include "ak_ao.h"
#include "ak_log.h"
}


class Ao
{
private:
    void set_param();
    int init();
    
public:
    Ao(/* args */);
    ~Ao();
    void print_playing_dot(void);
    int ao_handle_id;
    enum ak_audio_sample_rate sample_rate;
    enum ak_audio_channel_type channel_num;
};



#endif