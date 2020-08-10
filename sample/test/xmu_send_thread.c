#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "xmu_send_thread.h"
#include "xmu_common.h"
// #include "xmu_venc.h"

#include "ak_thread.h"
#include "ak_log.h"

#define SEVER_PORT 8888
char* SERVERIP = "127.0.0.1";

#define SPLIT_SIZE 1024

struct xmu_send_thread_data
{
    void *data;
    int data_len;
}send_data;

struct splited_data
{
    int data_len;
    unsigned char seq_no;
    unsigned char spl_no;
    unsigned char data[SPLIT_SIZE];  //1024 size
};
static unsigned char seq_count = 0;

static ak_pthread_t send_stream;
static ak_sem_t send_sem;

static struct sockaddr_in servaddr;
// static char sendbuf[SPLIT_SIZE];
static int sock;

void venc_udp_thread_sem_post();


void *send_thread(void *arg){
    ak_thread_set_name("send_thread");
    int ret;
    while (1)
    {
        ak_thread_sem_wait(&send_sem);
        // struct splited_data sp_data;
        // int i = 0;
        // int res_len = send_data.data_len;
        // int start_len = 0;
        // while(1){
        //     sp_data.spl_no = i;
        //     sp_data.seq_no = seq_count;
        //     sp_data.data_len = SPLIT_SIZE;
        //     if (res_len > SPLIT_SIZE){
        //         memccpy(sp_data.data, send_data.data, start_len, SPLIT_SIZE);
        //     } else {
        //         sp_data.data_len = res_len;
        //         memccpy(sp_data.data, send_data.data, start_len, res_len);
        //     }
        //     start_len += SPLIT_SIZE;
        //     res_len -= SPLIT_SIZE;
        //     i++;
        //     sendto(sock, (void*)&sp_data, sizeof(struct splited_data), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        // }
        // seq_count++;
        ak_print_normal(1, "thread send success\n");
        venc_udp_thread_sem_post(); //notify venc thread to start
    }
    
}
int send_thread_init(){
    //socket init

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        return FAILED;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SEVER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVERIP);

    ak_thread_sem_init(&send_sem, 0);
    return SUCCESS;
}
void send_thread_start(){
    ak_thread_create(&send_stream, send_thread, 0, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

void send_thread_set_data(void *data, int data_len){
    // send_data.data = data;
    // send_data.data_len = data_len;
    ak_thread_sem_post(&send_sem);
}

