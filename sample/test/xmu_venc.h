#ifndef XMU_VENC_H
#define XMU_VENC_H
#include "ak_vi.h"

void venc_set_param(void);
int venc_init(void);
void venc_start(void);
void venc_close(void);
void venc_thread_sem_post(void);
void enc_pair_set_source(struct video_input_frame *frame);
#endif