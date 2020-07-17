#ifndef XMU_VI_H
#define XMU_VI_H

void vi_set_param(void);
int vi_init(void);
int vi_get_one_frame(struct video_input_frame *frame, int fream_len);

#endif