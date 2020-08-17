#ifndef XMU_AI_H
#define XMU_AI_H

#include "BaseThread.h"
#include "DataBuff.h"

extern "C" {
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_thread.h"
#include "ak_log.h"
}

class Ai :public BaseThread
{
private:
    void set_param();
    int init();
    int get_frame_len(int encode_type, int sample_rate, int channel);
    void print_playing_dot(void);
    int ai_handle_id;
    int aenc_handle_id;
    int send_frame_end;

    enum ak_audio_sample_rate sample_rate;
    enum ak_audio_channel_type channel_num;
    enum ak_audio_type enc_type;
    // int save_time;        // set save time(ms)
    // char *save_path;    // set save path
public:
    Ai(/* args */);
    ~Ai();
    void start();
    void run();
    DataBuffer *dbf;
};

#endif