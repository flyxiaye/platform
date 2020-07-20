#ifndef XMU_RECV_THREAD_H
#define XMU_RECV_THREAD_H

int recv_thread_init();
void recv_thread_start();
void recv_thread_set_data(void *data, int data_len);

#endif