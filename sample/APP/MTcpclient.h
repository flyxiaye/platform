#ifndef MTCPCLIENT_H
#define MTCPCLIENT_H

#include "BaseThread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <DataBuff.h>

class MTcpclient :public BaseThread
{
private:
    int sockfd, n;
    unsigned char recvline[4096], sendline[4096];
    struct sockaddr_in servaddr;
    char *ip;
public:
    MTcpclient(/* args */);
    ~MTcpclient();
    void start();
    void run();
    void start_send();
    DataBuffer *dbf;
};



#endif