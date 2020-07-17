#ifndef XMU_VI_H
#define XMU_VI_H
#include "ak_common.h"
#include "ak_vi.h"

void vi_set_param(void);
int vi_init(void);
int vi_get_one_frame(struct video_input_frame *frame, int fream_len);
void vi_release_one_frame(struct video_input_frame *frame);
#endif
