#ifndef XMU_VDEC_H
#define XMU_VDEC_H

void vdec_set_param(void);
int vdec_init(void);
void vdec_start(void);
void vdec_close(void);
void vdec_set_vdec_data(unsigned char *data, int data_len);
void vdec_thread_sem_post(void);
void vdec_open_test();
#endif