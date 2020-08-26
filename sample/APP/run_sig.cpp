#include "run_sig.h"
#include "memory.h"
#include <iostream>
extern "C"{
#include "ak_common.h"
}

RunState::RunState(int sock, sockaddr_in *server, Dir dir)
{
    this->socket1 = sock;
    this->server = server;
    this->dir = dir;
    datbuf = new unsigned char[10];
}

RunState::~RunState()
{
    delete[] datbuf;
}

void RunState::run()
{
    
    if (dir == MSG_OUT){
        while(1){
            memcpy(datbuf, run_state, sizeof(int));
            sendto(socket1, datbuf, sizeof(int), 0, (struct sockaddr*)server, sizeof(sockaddr));
            ak_sleep_ms(50);
        }
    } else {
        bind(socket1, (struct sockaddr *)server, sizeof(sockaddr));
        struct sockaddr_in peeraddr;
        socklen_t peerlen;
        int n = 0;
        while(1)
        {
            n = recvfrom(socket1, datbuf, 10, 0, (sockaddr *)&peeraddr, &peerlen);
            if (n <= 0)
                continue;
            else if (n > 0){
                *run_state = *(int *)datbuf;
            }
        }
    }
}

static void *callback(void *arg)
{
    ((RunState*)arg)->run();
}

void RunState::start()
{
    BaseThread::start(callback);
}

