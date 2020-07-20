#ifndef XMU_SEND_THREAD_H
#define XMU_SEND_THREAD_H

int send_thread_init();
void send_thread_start();
void send_thread_set_data(void *data, int data_len);
#endif