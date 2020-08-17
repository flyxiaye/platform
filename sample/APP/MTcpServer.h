#ifndef MTCPSERVER_H
#define MTCPSERVER_H

#include "BaseThread.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "DataBuff.h"

class MTcpServer :public BaseThread
{
private:
    struct sockaddr_in *servaddr;
    int listenfd, connfd;
    unsigned char *buff;

public:
    enum {
        MAXLINE = 4096
    };
    MTcpServer(/* args */);
    MTcpServer(int port);
    ~MTcpServer();
    void start();
    void run();
    DataBuffer *dbf;
};



#endif