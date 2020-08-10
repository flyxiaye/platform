#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "xmu_recv_thread.h"
#include "xmu_common.h"

#include "ak_thread.h"
#include "ak_mem.h"

#define RECV_PORT 8888
static char* SERVERIP = "127.0.0.1";

#define SPLIT_SIZE 1024

struct xmu_recv_thread_data
{
    void *data;
    int data_len;
}recv_data;

struct splited_data
{
    int data_len;
    unsigned char seq_no;
    unsigned char spl_no;
    unsigned char data[SPLIT_SIZE];  //1024 size
};

static ak_pthread_t recv_stream;
static ak_sem_t recv_sem;

static struct sockaddr_in servaddr;
static char recvbuf[SPLIT_SIZE+10];
static int sock;

void *recv_thread(void *arg){
    ak_thread_set_name("recv_thread");
    int ret;
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;
    while (1)
    {
        // ak_thread_sem_wait(&recv_sem);
        int seq_no = -1;
        int recv_ack = 0;
        int dat_len = 0;
        struct splited_data * recv_split;
        memset(&recv_data, 0, sizeof(struct xmu_recv_thread_data));
        recv_data.data = ak_mem_alloc(MODULE_ID_THREAD, SPLIT_SIZE * 100);
        do{
            peerlen = sizeof(peeraddr);
            memset(recvbuf, 0, sizeof(recvbuf));
            n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                        (struct sockaddr *)&peeraddr, &peerlen);
            if (n <= 0){
                continue;
            }
            recv_split = (struct splited_data *)recvbuf;
            dat_len += recv_split->data_len;
            if (recv_split->spl_no == 0 && seq_no == -1){
                memcpy(recv_data.data, recv_split->data, recv_split->data_len);
            } else if (seq_no != -1){
                memcpy(recv_data.data+recv_split->spl_no*SPLIT_SIZE, recv_split->data, recv_split->data_len);
                if (recv_split->data_len < SPLIT_SIZE)
                {
                    recv_ack = 1;
                    break;
                }
            } else
            {
                break;
            }
        } while(1);
        if (recv_ack)
        {
            recv_data.data_len = dat_len;
            //TODO:
        }
    }
    
}
int recv_thread_init(){
    //socket init

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        return FAILED;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(RECV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("listen %d port\n",RECV_PORT);
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return FAILED;

    ak_thread_sem_init(&recv_sem, 0);
    return SUCCESS;
}
void recv_thread_start(){
    ak_thread_create(&recv_stream, recv_thread, 0, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

void recv_thread_set_data(void *data, int data_len){
    recv_data.data = data;
    recv_data.data_len = data_len;
    ak_thread_sem_post(&recv_sem);
}