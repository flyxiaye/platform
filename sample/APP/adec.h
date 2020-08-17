#ifndef ADEC_H
#define ADEC_H

#include "xmu_ao.h"
#include "BaseThread.h"
#include "DataBuff.h"
extern "C" {
#include "ak_adec.h"
#include "ak_common.h"
#include "ak_ao.h"
}

class Adec :public BaseThread
{
private:
    void set_param();
    int init();
public:
    Adec(Ao *ao);
    ~Adec();
    void run();
    void start();

    int adec_handle_id;
    enum ak_audio_type dec_type;
    Ao *ao;
};

class AdecSend :public BaseThread
{
private:
    int adec_handle_id;
    int send_frame_end;
public:
    AdecSend(Adec *ad);
    ~AdecSend();
    void run();
    void start();
    
    Adec *adec;
    DataBuffer *dbf;
};





#endif