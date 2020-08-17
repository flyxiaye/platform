#include "MTcpclient.h"
#include <memory.h>
#include <stdio.h>

MTcpclient::MTcpclient()
{
    MTcpclient("127.0.0.1", 8000);
}

MTcpclient::MTcpclient(const char * ip)
{
    MTcpclient(ip, 8000);
}

MTcpclient::MTcpclient(const char * ip, int port)
{
    // ip = (char *)"192.168.1.9";
    // ip = (char *)"127.0.0.1";
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("create socket error\n");
        return;
    }

    // memset(&servaddr, 0, sizeof(servaddr));
    servaddr = new struct sockaddr_in;
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons(port);
    if( inet_pton(AF_INET, ip, &servaddr->sin_addr) <= 0){
        printf("inet_pton error for\n");
        return;
    }

    if( connect(sockfd, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)) < 0){
        printf("connect error\n");
        return;
    }
    recvline = new unsigned char[4096];
    sendline = new unsigned char[4096];
}

MTcpclient::~MTcpclient()
{
    delete[] recvline;
    delete[] sendline;
    delete servaddr;
    close(sockfd);
}

void MTcpclient::run()
{
    while(1)
    {
        int data_len;
        dbf->rb_read(sendline, 4096, &data_len);
        if( data_len > 0 && send(sockfd, sendline, data_len, 0) < 0)
        {
            printf("send msg error\n");
            break;
        }
    }

    // for test
    // while (1)
    // {
    //     if( send(sockfd, "sendline", strlen("sendline"), 0) < 0)
    //     {
    //         printf("send msg error\n");
    //         break;
    //     }
    // }
}

static void * callback(void *arg)
{
    ((MTcpclient*)arg)->run();
}

void MTcpclient::start()
{
    BaseThread::start(callback);
}

// void MTcpclient::start_send(unsigned char *data, int data_len)
// {
//     this->data_p = data;
//     this->data_len = data_len;
//     ak_thread_sem_post(&sem);
// }
